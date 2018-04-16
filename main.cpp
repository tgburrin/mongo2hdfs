#include <iostream>
#include <csignal>
#include <ctime>

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

#define DEFAULT_NUMBER_RECORD_LIMIT 10000
#define DEFAULT_TIME_RECORD_LIMIT 30

using namespace std;

// Ugh a global, but the signal handler will need this
bool running = true;

void flushUpdates(vector<MongoShardInfo> *shards, unordered_map<string, HdfsFile*> *fileMap, time_t *lastFlushTime) {
	// lock all files
	for ( auto it = fileMap->begin(); it != fileMap->end(); ++it ) {
		HdfsFile *fd = it->second;
		fd->lck->lock();
		fd->flushFile();
	}

	vector<MongoShardInfo>::iterator it;
	for( it = shards->begin(); it != shards->end(); ++it )
		it->saveBookmark();

	for ( auto it = fileMap->begin(); it != fileMap->end(); ++it ) {
		HdfsFile *fd = it->second;
		fd->lck->unlock();
	}

	time(lastFlushTime);
}

void flushByTimeThread(bool *running, time_t *lastFlushTime, vector<MongoShardInfo> *shards, unordered_map<string, HdfsFile*> *fileMap) {
	time_t currentTime;

	while( *running ) {
		time(&currentTime);
		if ( currentTime - *lastFlushTime >= DEFAULT_TIME_RECORD_LIMIT ) {
			flushUpdates(shards, fileMap, lastFlushTime);
			cout << "Flushing updates based on time" << endl << flush;
		}
		usleep(500);
	}
}

void consumeEvents(bool *running,
		time_t *lastFlushTime,
		uint64_t *messageCounter,
		mutex *messageCounterLock,
		vector<MongoShardInfo> *shards,
		MongoShardInfo *shardInfo,
		bool initializeFromOplogStart,
		unordered_map<string, HdfsFile*> *fileMap,
		HdfsFileFactory *f,
		mutex *fileCreatorLock) {

	MongoOplogClient mc = MongoOplogClient(shardInfo->getClusterURI(), shardInfo->getShardURI());

	MongoMessage *m = NULL;

	while ( *running ) {
		while( *running && mc.readOplogEvent(shardInfo->getBookmark(), initializeFromOplogStart) ) {
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

			fd->lck->lock();
			fd->writeToFile(m->message);
			fd->lck->unlock();

			shardInfo->updateBookmark(m->timestamp, m->txnoffset);

			messageCounterLock->lock();
			(*messageCounter)++;
			if ( (*messageCounter) >= DEFAULT_NUMBER_RECORD_LIMIT ) {
				try {
					flushUpdates(shards, fileMap, lastFlushTime);
				} catch (ApplicationException *e) {
					cerr << e->what() << endl << flush;
					exit(EXIT_FAILURE);
				}
				(*messageCounter) = 0;
			}
			messageCounterLock->unlock();

			delete m;
			m = NULL;
		}
		this_thread::yield();
	}
	if ( m != NULL )
		delete m;
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
	time_t lastFlushTime;
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

	// Register the signal that will allow us to clean up and shutdown
	// CTRL-C for now
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

	vector<MongoShardInfo> shards;
	try {
		shards = MongoUtilities::getShardUris(cfg->getMongosURI(), &book);
	} catch ( ApplicationException &e ) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	} catch ( MongoException &e ) {
		cerr << e.what() << endl;
		exit(EXIT_FAILURE);
	}

	uint64_t messageCounter = 0;
	time(&lastFlushTime);
	mutex messageCounterLock;

	// Start the by time thread
	thread timeflushThread(flushByTimeThread, &running, &lastFlushTime, &shards, fileMap);

	vector<thread> children;
	for( auto it = shards.begin(); it != shards.end(); ++it) {
		children.push_back(thread(consumeEvents,
				&running,
				&lastFlushTime,
				&messageCounter,
				&messageCounterLock,

				&shards,
				&(*it),
				cfg->getMongoInitFromStart(),

				fileMap,
				fileCreator,
				&fileCreatorLock));
	}

	// Block the main thread until children start exiting
	for( uint i = 0; i < children.size(); i++ )
		if( children.at(i).joinable() )
			children.at(i).join();

	timeflushThread.join();

	cout << "Flushing outstanding messages and updating bookmarks" << endl;
	flushUpdates(&shards, fileMap, &lastFlushTime);

	for ( auto it = fileMap->begin(); it != fileMap->end(); ++it)
		delete it->second;

	mongoc_cleanup();

	delete fileMap;
	delete fileCreator;
	delete cfg;

	cout << "Done" << endl;
}
