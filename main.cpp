#include <iostream>
#include <csignal>

#include <thread>
#include <mutex>

#include <vector>
#include <unordered_map>

#include <unistd.h>

// MAC OSX specific include
#ifdef __APPLE_CC__
#include <libgen.h>
#endif

#include <mongoc.h>

#include "common/ProcessCfg.h"
#include "common/BookmarkManager.h"

#include "mongo/MongoUtilities.h"
#include "mongo/MongoOplogClient.h"
#include "mongo/MongoMessage.h"
#include "mongo/MongoShardInfo.h"

#include "hdfs/HdfsFile.h"
#include "hdfs/HdfsFileFactory.h"

using namespace std;

// Ugh a global, but the signal handler will need this
bool running = true;

void consumeEvents(bool *running,
		string clusterURI,
		MongoShardInfo shardInfo,
		bool initializeFromOplogStart,
		unordered_map<string, HdfsFile*> *fileMap,
		HdfsFileFactory *f,
		mutex *fileCreatorLock) {

	MongoOplogClient mc = MongoOplogClient(clusterURI, shardInfo.getShardURI());

	MongoMessage *m = NULL;

	while ( *running ) {
		while(mc.readOplogEvent(shardInfo.getBookmark(), initializeFromOplogStart)) {
			if ( m != NULL ) {
				delete m;
				m = NULL;
			}

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
			if ( fd->writeToFile(m->message) )
				shardInfo.updateBookmark(m->timestamp, m->txnoffset);
			fd->lck->unlock();
		}
		this_thread::yield();
	}
	if ( m != NULL ) {
		shardInfo.updateBookmark(m->timestamp, m->txnoffset);
		delete m;
		m = NULL;
	}
}

void gracefulShutdown( int signum ) {
	switch(signum) {
	case SIGINT:
		cout << "Shutting down" << endl << flush;
		running = false;
		break;
	default:
		cout << "Received signal #" << signum << endl << flush;
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

	signal(SIGINT, &gracefulShutdown);

	// Shared by all threads since they will all have records from sharded collections
	unordered_map<string, HdfsFile*> *fileMap = new unordered_map<string, HdfsFile*>();

	// The class for creating new filehandles / collection
	HdfsFileFactory *fileCreator = new HdfsFileFactory(cfg);

	mutex fileCreatorLock;

	mongoc_init ();

	// Created separately for now as we might want a dedicated writer thread that will update
	// the shard bookmarks
	BookmarkManager book = BookmarkManager(cfg);

	string clusterURI = cfg->getMongosURI();
	vector<MongoShardInfo> shards;
	try {
		shards = MongoUtilities::getShardUris(clusterURI, &book);
	} catch ( ApplicationException &e ) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	} catch ( MongoException &e ) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}

	vector<thread> children;
	for( auto shard : shards )
		children.push_back(thread(consumeEvents, &running, clusterURI, shard, cfg->getMongoInitFromStart(), fileMap, fileCreator, &fileCreatorLock));

	for( uint i = 0; i < children.size(); i++ )
		if( children.at(i).joinable() )
			children.at(i).join();

	unordered_map<string, HdfsFile*>::const_iterator i;
	for ( i = fileMap->begin(); i != fileMap->end(); ++i) {
		delete i->second;
	}

	mongoc_cleanup();

	delete fileMap;
	delete fileCreator;
	delete cfg;
}
