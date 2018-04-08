/*
 * MongoOplogClient.cpp
 *
 *  Created on: Mar 31, 2018
 *      Author: tgburrin
 */

#include "MongoOplogClient.h"

MongoOplogClient::MongoOplogClient(string cURI, string sURI) {
	clusterURI = cURI;
	shardURI = sURI;
	connectToMongo();
}

MongoOplogClient::~MongoOplogClient() {
	close();
}

void MongoOplogClient::startCursor() {
	bson_t *filter = BCON_NEW(
			"fromMigrate", "{",
				"$ne", BCON_BOOL(true),
			"}",
			"ns", "{",
				"$ne", "",
			    "$regex", "^(?!(config\\.|admin\\.))(?!.*(\\.\\$cmd$))",  // Revmoe admin db/config db/and commands
			"}",
			"op", "{",
				"$ne", "n", // Remove notifications
			"}");
	bson_t *options = BCON_NEW("tailable", BCON_BOOL(true), "maxAwaitTimeMS", BCON_INT64(10));

	cur = mongoc_collection_find_with_opts(oplogCollection, filter, options, NULL);

	bson_destroy(filter);
	bson_destroy(options);
}

bool MongoOplogClient::readOplogEvent() {
	if ( cur == NULL )
		startCursor();

	return mongoc_cursor_next(cur, &oplogEvent);
}

void MongoOplogClient::connectToMongo() {
	cout << "Connecting to " << shardURI << endl << flush;
	if ( (cli = mongoc_client_new(shardURI.c_str())) == NULL )
		throw MongoException("Unable to connect to mongo shard: "+shardURI);

	if ( (cluster = mongoc_client_new(clusterURI.c_str())) == NULL )
		throw MongoException("Unable to connect to mongo cluster: "+clusterURI);


	bson_t *stat = bson_new();
	mongoc_client_get_server_status(cli, mongoc_read_prefs_new(MONGOC_READ_PRIMARY), stat, NULL);
	bson_iter_init(&docIterator, stat);
	string hostname = "unknown";
	string version = "unknown";

	if ( bson_iter_init_find(&docIterator, stat, "host") )
		hostname = bson_iter_utf8(&docIterator, 0);
	if ( bson_iter_find(&docIterator, "version") )
		version = bson_iter_utf8(&docIterator, 0);

	cout << "Connected to " << hostname << " (v"  << version << ")" << endl << flush;

	bson_destroy(stat);

	oplogCollection = mongoc_client_get_collection(cli, "local", "oplog.rs");

	cout << "There are " << mongoc_collection_count(oplogCollection, MONGOC_QUERY_NONE, bson_new(), 0, 0, NULL, NULL) << "  events" << endl << flush;
}

bson_t *MongoOplogClient::lookupDocument(string dbname, string collection, bson_t *id) {
	bson_t *rv = NULL;
	const bson_t *doc = bson_new();

	mongoc_collection_t *c = mongoc_client_get_collection(cluster, dbname.c_str(), collection.c_str());
	mongoc_cursor_t *f = mongoc_collection_find_with_opts(c, id, NULL, NULL);

	while (mongoc_cursor_next (f, &doc)) {
		// This is less than ideal, but since this should be finding only one document it shouldn't be too much of a problem
		if ( rv != NULL )
			bson_destroy(rv);

		rv = bson_copy(doc);
	}

	mongoc_cursor_destroy(f);
	mongoc_collection_destroy(c);

	return rv;
}

void MongoOplogClient::close() {
	if ( oplogCollection )
		mongoc_collection_destroy(oplogCollection);

	if ( cli )
		mongoc_client_destroy(cli);

	if ( cluster )
		mongoc_client_destroy(cluster);
}

MongoMessage *MongoOplogClient::getEvent() {
	string rv;
	ostringstream errStream;

	string dbOperation;
	string dbNamespace;

	uint32_t timestamp;
	uint32_t txnoffset;

	bson_iter_init(&docIterator, oplogEvent);
	while (bson_iter_next (&docIterator)) {
		string k = bson_iter_key (&docIterator);
		if ( k.compare("op") == 0 )
			dbOperation = bson_iter_utf8(&docIterator, NULL);
		else if ( k.compare("ns") == 0 )
			dbNamespace = bson_iter_utf8(&docIterator, NULL);
		else if ( k.compare("ts") == 0 )
			bson_iter_timestamp(&docIterator, &timestamp, &txnoffset);
	}

	/*
	if ( bson_iter_init_find(&docIterator, oplogEvent, "ts") )
		bson_iter_timestamp(&docIterator, &timestamp, &txnoffset);
	*/

	// Check to make sure we have the basics
	if ( dbOperation.empty() || dbNamespace.empty() ) {
		errStream.clear();
		errStream << "Operation and Namespace must be populated: " << endl
				<< bson_as_canonical_extended_json(oplogEvent, NULL) << endl;
		throw MongoException(errStream.str());
	}

	auto it = find(validOperations.begin(), validOperations.end(), dbOperation);
	if ( it == validOperations.end() ) {
		errStream.clear();
		errStream << "Unknown operation " << dbOperation << " for message: " << endl
				<< bson_as_canonical_extended_json(oplogEvent, NULL) << endl;
		throw MongoException(errStream.str());
	}

	bson_t *underlyingDocument = NULL;
	uint32_t msglen;
	const uint8_t *data;

	// Updates hide the search criteria in 'o2' and that's what we need to look up the doc (now out of date)
	dbOperation.compare("u") == 0 ?
			bson_iter_init_find(&docIterator, oplogEvent, "o2") :
			bson_iter_init_find(&docIterator, oplogEvent, "o");

	bson_iter_document(&docIterator, &msglen, &data);

	if ( dbOperation.compare("u") == 0 ) {
		string dbname = dbNamespace.substr(0, dbNamespace.find("."));
		string colname = dbNamespace.substr(dbNamespace.find(".")+1, dbNamespace.size());

		bson_t id;
		bson_init_static(&id, data, msglen);
		underlyingDocument = lookupDocument(dbname, colname, &id);
		bson_destroy(&id);
	} else {
		bson_t doc;
		bson_init_static(&doc, data, msglen);
		underlyingDocument = &doc;
	}

	bson_t *msg = BCON_NEW("operation", BCON_UTF8(dbOperation.c_str()), "document", BCON_DOCUMENT(underlyingDocument));

	// This must be assigned to a char * and then to a string so that the pointer may be freed later on
	// otherwise this results in a memory leak
	char *cmsg = bson_as_canonical_extended_json(msg, NULL);

	MongoMessage *m = new MongoMessage();
	m->message = cmsg;
	m->operation = dbOperation;
	m->timestamp = timestamp;
	m->txnoffset = txnoffset;

	bson_destroy(underlyingDocument);
	bson_destroy(msg);
	bson_free(cmsg);

	return m;
}
