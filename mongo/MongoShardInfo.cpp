/*
 * MongoShardInfo.cpp
 *
 *  Created on: Apr 10, 2018
 *      Author: tburrington
 */

#include "MongoShardInfo.h"

MongoShardInfo::MongoShardInfo(string name, string uri, BookmarkManager *book) : shardName(name), shardURI(uri), bookmark(book) {
	timestamp = 0;
	increment = 0;
	refreshBookmark();
}
MongoShardInfo::~MongoShardInfo() {
	/* This causes a pointer to be freed twice
	 * this should be verified in some way
	if ( findTs != NULL )
		bson_destroy(findTs);
	*/
}

string MongoShardInfo::getShardName() {
	return shardName;
}

string MongoShardInfo::getShardURI() {
	return shardURI;
}

string MongoShardInfo::getClusterURI() {
	return clusterURI;
}

bson_t *MongoShardInfo::getBookmark() {
	return findTs;
}

void MongoShardInfo::refreshBookmark() {
	bool rv = bookmark->getTimestampBookmarkValues(shardName, &timestamp, &increment);
	if ( rv )
		findTs = BCON_NEW("ts", "{", "$gt", BCON_TIMESTAMP(timestamp, increment), "}");
}

void MongoShardInfo::updateBookmark(uint32_t ts, uint32_t inc) {
	timestamp = ts;
	increment = inc;
}

void MongoShardInfo::saveBookmark() {
	bookmark->setBookmark(shardName, &timestamp, &increment);
}
