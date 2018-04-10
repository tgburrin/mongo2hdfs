#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <list>
#include <unordered_map>

#include <unistd.h>

// MAC OSX specific include
#ifdef __APPLE_CC__
#include <libgen.h>
#endif

#include <mongoc.h>

#include "common/ProcessCfg.h"
#include "mongo/MongoUtilities.h"
#include "mongo/MongoOplogClient.h"
#include "mongo/MongoMessage.h"
#include "mongo/MongoShardInfo.h"

#include "hdfs/HdfsFile.h"
#include "hdfs/HdfsFileFactory.h"

using namespace std;

void consumeEvents(bool *running,
		string clusterURI,
		MongoShardInfo shardInfo,
		unordered_map<string, HdfsFile*> *fileMap,
		HdfsFileFactory *f,
		mutex *fileCreatorLock,
		uint32_t threadNum) {

	MongoOplogClient mc = MongoOplogClient(clusterURI, shardInfo.shardURI);

	while ( *running ) {
		while(mc.readOplogEvent()) {
			MongoMessage *m = NULL;
			try {
				 m = mc.getEvent();
			} catch (MongoException *e) {
				cerr << e->what() << endl << flush;
				exit(EXIT_FAILURE);
			}

			HdfsFile *fd = NULL;

			// A large area to lock up, but the alternative (a dirty read) does not work
			fileCreatorLock->lock();
			unordered_map<string, HdfsFile*>::const_iterator fdFind = fileMap->find(m->dbNamespace);
			if ( fdFind == fileMap->end() ) {
				try {
					fd = f->getFile(m->dbNamespace+".txt");
				} catch ( HdfsFileException &e ) {
					cerr << e.what() << endl << flush;
					exit(EXIT_FAILURE);
				}
				pair<string, HdfsFile *> newFileHandle(m->dbNamespace, fd);

				fileMap->insert(newFileHandle);
			} else {
				fd = fdFind->second;
			}
			fileCreatorLock->unlock();

			if ( fd == NULL ) {
				cerr << "Unable to open fd" << endl << flush;
				exit(EXIT_FAILURE);
			}

			fd->lck->lock();
			fd->writeToFile(m->message);
			fd->lck->unlock();

			delete m;
		}
	}
}

string PrintUsage( string program_name ) {
	stringstream message;

	message << "Usage: " << program_name << " <options>" << endl;
	message << "\t-c <config file> (required)" << endl;
	message << "\t-h this help menu" << endl;

	return message.str();
}

int main (int argc, char **argv) {
	bool running = true;
	string configFile;

	int opt;
	extern char *optarg;

	while ((opt = getopt(argc, argv, "hc:k")) != -1) {
		switch (opt) {
			case 'c':
				configFile = optarg;
				break;
			case 'h':
				cout << PrintUsage(basename(argv[0]));
				exit(EXIT_SUCCESS);
			default:
				cerr << "Unknown option provided" << endl;
				cerr << PrintUsage(basename(argv[0]));
				exit(EXIT_FAILURE);
		}
	}

	if ( configFile.empty() ) {
		cerr << "-c is a required option" << endl;
		cerr << PrintUsage(basename(argv[0]));
		exit(EXIT_FAILURE);
	}

	ProcessCfg *cfg;
	try {
		cfg = new ProcessCfg(configFile);
	} catch (ApplicationException *e) {
		cerr << e->what() << endl << flush;
		exit(EXIT_FAILURE);
	}

	unordered_map<string, HdfsFile*> *fileMap = new unordered_map<string, HdfsFile*>();

	HdfsFileFactory *fileCreator = new HdfsFileFactory(cfg->getHdfsUsername(), cfg->getHdfsNameNode(), cfg->getHdfsBasePath());

	mutex fileCreatorLock;

	mongoc_init ();

	string clusterURI = cfg->getMongosURI();
	vector<MongoShardInfo> shards = MongoUtilities::getShardUris(clusterURI);

	uint32_t childCount = 1;
	vector<thread> children;
	for( auto shard : shards ) {
		children.push_back(thread(consumeEvents, &running, clusterURI, shard, fileMap, fileCreator, &fileCreatorLock, childCount));
		childCount++;
	}

	for( uint i = 0; i < children.size(); i++ )
		if( children.at(i).joinable() )
			children.at(i).join();

	mongoc_cleanup();

	delete fileMap;
	delete fileCreator;
	delete cfg;
}
