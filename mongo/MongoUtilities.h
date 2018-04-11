/*
 * MongoUtilities.h
 *
 *  Created on: Mar 27, 2018
 *	  Author: tgburrin
 */

#ifndef MONGO_MONGOUTILITIES_H_
#define MONGO_MONGOUTILITIES_H_

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <mongoc.h>
#include <bson.h>

using namespace std;

#include "MongoException.h"
#include "MongoShardInfo.h"
#include "../common/BookmarkManager.h"

class MongoShardInfo;
class MongoUtilities {
private:
	MongoUtilities();
	virtual ~MongoUtilities();

public:
	static vector<MongoShardInfo> getShardUris(string,BookmarkManager *);
};

#endif /* MONGO_MONGOUTILITIES_H_ */
