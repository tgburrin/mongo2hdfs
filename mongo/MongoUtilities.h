/*
 * MongoUtilities.h
 *
 *  Created on: Mar 27, 2018
 *	  Author: tgburrin
 */

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <mongoc.h>
#include <bson.h>

using namespace std;

#ifndef MONGO_MONGOUTILITIES_H_
#define MONGO_MONGOUTILITIES_H_

class MongoUtilities {
public:
	MongoUtilities();
	virtual ~MongoUtilities();
	static vector<string> getShardUris(string);
};

#endif /* MONGO_MONGOUTILITIES_H_ */
