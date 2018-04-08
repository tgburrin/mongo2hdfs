/*
 * MongoException.cpp
 *
 *  Created on: Apr 2, 2018
 *	  Author: tgburrin
 */

#include "MongoException.h"

MongoException::MongoException(string msg): message(msg) {}

MongoException::MongoException(const char *msg): message(msg) {}

MongoException::~MongoException() {}

const char* MongoException::what()
{
	return message.c_str();
}
