/*
 * MongoOplogClient.h
 *
 *  Created on: Mar 31, 2018
 *	  Author: tgburrin
 */

#ifndef MONGO_MONGOOPLOGCLIENT_H_
#define MONGO_MONGOOPLOGCLIENT_H_

#include <iostream>
#include <sstream>
#include <cstring>
#include <ctime>
#include <list>

#include <mongoc.h>
#include <bson.h>

#include "MongoMessage.h"
#include "MongoException.h"

using namespace std;

class MongoOplogClient {
private:
	string clusterURI;
	string shardURI;

	mongoc_client_t *cluster = NULL;
	mongoc_client_t *cli = NULL;

	mongoc_collection_t *oplogCollection = NULL;

	mongoc_cursor_t *cur = NULL;

	bson_iter_t docIterator;

	list<string> validOperations = { "i", "u", "d" };

	const bson_t *oplogEvent;

	void connectToMongo(void);
	void startCursor(bson_t *);
	bson_t *lookupDocument(string, string, bson_t *);
	void close();

public:
	MongoOplogClient(string, string);
	virtual ~MongoOplogClient();

	bool readOplogEvent(bson_t *);
	MongoMessage *getEvent();
};

#endif /* MONGO_MONGOOPLOGCLIENT_H_ */
