/*
 * MongoUtilities.cpp
 *
 *  Created on: Mar 27, 2018
 *	  Author: tgburrin
 */

#include "MongoUtilities.h"

MongoUtilities::MongoUtilities() {}
MongoUtilities::~MongoUtilities() {}

vector<MongoShardInfo> MongoUtilities::getShardUris(string mongosURI, BookmarkManager *book) {
	bson_error_t error;

	vector<MongoShardInfo> rv;

	mongoc_client_t *client = mongoc_client_new(mongosURI.c_str());

	if (!mongoc_client_set_appname(client, "test-client") )
		throw MongoException("Could not set appname");

	mongoc_collection_t *shards = mongoc_client_get_collection(client, "config", "shards");

	bson_t *query = bson_new(); // no filtering
	mongoc_cursor_t *c = mongoc_collection_find_with_opts(shards, query, NULL, NULL);

	const bson_t *doc;
	while (mongoc_cursor_next (c, &doc)) {
		bson_iter_t i;

		if(bson_iter_init_find(&i, doc, "host")) {
			istringstream ins(bson_iter_utf8(&i, NULL));

			vector<string> p;
			string s;

			while(getline(ins, s, '/'))
				p.push_back(s);

			MongoShardInfo ms(p[0], "mongodb://"+p[1]+"/?replicaSet="+p[0], book);
			ms.clusterURI = mongosURI;
			rv.push_back(ms);
		}
	}

	if (mongoc_cursor_error(c, &error))
		throw MongoException(error.message);

	bson_destroy(query);
	mongoc_collection_destroy(shards);

	mongoc_client_destroy(client);

	return rv;
}
