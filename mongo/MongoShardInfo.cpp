/*
 * MongoShardInfo.cpp
 *
 *  Created on: Apr 10, 2018
 *      Author: tburrington
 */

#include "MongoShardInfo.h"

MongoShardInfo::MongoShardInfo(string name, string uri) : shardName(name), shardURI(uri) {}
MongoShardInfo::~MongoShardInfo() {}

