/*
 * MongoShardInfo.h
 *
 *  Created on: Apr 10, 2018
 *      Author: tburrington
 */

#ifndef MONGO_MONGOSHARDINFO_H_
#define MONGO_MONGOSHARDINFO_H_

#include <iostream>

using namespace std;

class MongoShardInfo {
public:
	string shardName;
	string shardURI;

	MongoShardInfo(string, string);
	virtual ~MongoShardInfo();
};

#endif /* MONGO_MONGOSHARDINFO_H_ */
