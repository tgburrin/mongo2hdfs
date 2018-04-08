/*
 * MongoMessage.h
 *
 *  Created on: Apr 7, 2018
 *      Author: tgburrin
 */

#ifndef MONGO_MONGOMESSAGE_H_
#define MONGO_MONGOMESSAGE_H_

#include <iostream>
#include <cstdint>

using namespace std;

class MongoMessage {
public:
	string operation;
	string message;
	string dbname;

	uint32_t timestamp;
	uint32_t txnoffset;

	MongoMessage();
	virtual ~MongoMessage();
};

#endif /* MONGO_MONGOMESSAGE_H_ */
