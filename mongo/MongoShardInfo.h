/*
 * MongoShardInfo.h
 *
 *  Created on: Apr 10, 2018
 *      Author: tburrington
 */

#ifndef MONGO_MONGOSHARDINFO_H_
#define MONGO_MONGOSHARDINFO_H_

#include <iostream>

#include <bson.h>

#include "../common/BookmarkManager.h"

using namespace std;

class MongoShardInfo {
private:
	friend class MongoUtilities;

	string shardName;
	string shardURI;

	uint32_t timestamp;
	uint32_t increment;

	BookmarkManager *bookmark;

	bson_t *findTs =  NULL;

	MongoShardInfo(string, string, BookmarkManager *);
	void refreshBookmark();

public:
	virtual ~MongoShardInfo();

	string getShardName();
	string getShardURI();

	bson_t *getBookmark();
	void updateBookmark(uint32_t, uint32_t);
};

#endif /* MONGO_MONGOSHARDINFO_H_ */
