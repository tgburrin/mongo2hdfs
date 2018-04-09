#include <iostream>
#include <string>
#include <thread>
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

#include "hdfs/HdfsFile.h"

using namespace std;

void consumeEvents(bool *running, string clusterURI, string shardURI, unordered_map<string, HdfsFile> fileMap, HdfsFile *f) {
	MongoOplogClient mc = MongoOplogClient(clusterURI, shardURI);

	while ( *running ) {
		while(mc.readOplogEvent()) {
			MongoMessage *m = NULL;
			try {
				 m = mc.getEvent();
			} catch (MongoException *e) {
				cerr << e->what() << endl << flush;
				exit(EXIT_FAILURE);
			}

			f->lck->lock();
			f->writeToFile(m->message);
			f->lck->unlock();

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

	unordered_map<string, HdfsFile> fileMap;

	HdfsFile *f = new HdfsFile(cfg->getHdfsUsername(), cfg->getHdfsNameNode());
	try {
		f->setBasePath(cfg->getHdfsBasePath());
		f->openFile("testFile.txt");
	} catch (HdfsFileException *e) {
		cerr << e->what() << endl;
		exit(EXIT_FAILURE);
	}

	mongoc_init ();

	string clusterURI = cfg->getMongosURI();
	vector<string> shards = MongoUtilities::getShardUris(clusterURI);

	vector<thread> children;
	for( auto shard : shards )
		children.push_back(thread(consumeEvents, &running, clusterURI, shard, fileMap, f));

	for( uint i = 0; i < children.size(); i++ )
		if( children.at(i).joinable() )
			children.at(i).join();

	mongoc_cleanup();

	delete cfg;
}
