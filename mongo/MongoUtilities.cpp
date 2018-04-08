/*
 * MongoUtilities.cpp
 *
 *  Created on: Mar 27, 2018
 *	  Author: tgburrin
 */

#include "MongoUtilities.h"

MongoUtilities::MongoUtilities() {
}

MongoUtilities::~MongoUtilities() {
}

vector<string> MongoUtilities::getShardUris(string mongosURI) {
	bson_error_t error;

	vector<string> rv;

	mongoc_client_t *client = mongoc_client_new(mongosURI.c_str());

	if (!mongoc_client_set_appname(client, "test-client") ) {
		cerr << "Could not set app name" << endl;
	}


	mongoc_collection_t *shards = mongoc_client_get_collection(client, "config", "shards");

	bson_t *query = bson_new(); // no filtering
	mongoc_cursor_t *c = mongoc_collection_find_with_opts(shards, query, NULL, NULL);

	const bson_t *doc;
	while (mongoc_cursor_next (c, &doc)) {
		bson_iter_t i;
		bson_iter_init(&i, doc);

		if(bson_iter_find(&i, "host")) {
			istringstream ins(bson_iter_utf8(&i, NULL));

			vector<string> p;
			string s;

			while(getline(ins, s, '/'))
				p.push_back(s);

			rv.push_back("mongodb://"+p[1]+"/?replicaSet="+p[0]);
		}
	}

	if (mongoc_cursor_error(c, &error))
		cerr << error.message << endl;

	bson_destroy(query);
	mongoc_collection_destroy(shards);

	mongoc_client_destroy(client);

	return rv;
}
