/*
 * MongoShardInfo.cpp
 *
 *  Created on: Apr 10, 2018
 *      Author: tburrington
 */

#include "MongoShardInfo.h"

MongoShardInfo::MongoShardInfo(string name, string uri, BookmarkManager *book) : shardName(name), shardURI(uri), bookmark(book) {
	refreshBookmark();
}
MongoShardInfo::~MongoShardInfo() {
	if ( findTs != NULL )
		bson_destroy(findTs);
}

string MongoShardInfo::getShardName() {
	return shardName;
}

string MongoShardInfo::getShardURI() {
	return shardURI;
}

bson_t *MongoShardInfo::getBookmark() {
	return findTs;
}

void MongoShardInfo::refreshBookmark() {
	bool rv = bookmark->getTimestampBookmarkValues(shardName, &timestamp, &increment);
	if ( rv )
		findTs = BCON_NEW("ts", "{", "$gt", BCON_TIMESTAMP(timestamp, increment), "}");
}
