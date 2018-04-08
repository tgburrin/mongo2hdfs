#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <list>
#include <unordered_map>

#include <unistd.h>

#include <mongoc.h>

#include "common/ProcessCfg.h"
#include "mongo/MongoUtilities.h"
#include "mongo/MongoOplogClient.h"
#include "mongo/MongoMessage.h"

#include "hdfs/HdfsFile.h"

using namespace std;

void consumeEvents(string clusterURI, string shardURI, unordered_map<string, HdfsFile> fileMap) {
	MongoOplogClient mc = MongoOplogClient(clusterURI, shardURI);

	while(mc.readOplogEvent()) {
		MongoMessage *m = mc.getEvent();

		delete m;
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

    ProcessCfg cfg = ProcessCfg(configFile);

    unordered_map<string, HdfsFile> fileMap;

    /*
    HdfsFile *f = new HdfsFile(cfg.getHdfsUsername(), cfg.getHdfsNameNode());
	try {
		f->setBasePath(cfg.getHdfsBasePath());

		f->openFile("myTestFile.txt");
		f->writeToFile("This is a bunch of text\n");

    } catch (HdfsFileException e) {
    	cerr << e.what() << endl;
    	exit(EXIT_FAILURE);
    }
	delete f;
    exit(EXIT_SUCCESS);
	*/

	mongoc_init ();

	string clusterURI = cfg.getMongosURI();
    vector<string> shards = MongoUtilities::getShardUris(clusterURI);

    vector<thread> children;
    for( auto shard : shards )
    	children.push_back(thread(consumeEvents, clusterURI, shard, fileMap));

    for( uint i = 0; i < children.size(); i++ )
    	if( children.at(i).joinable() )
    		children.at(i).join();

	mongoc_cleanup();
}
