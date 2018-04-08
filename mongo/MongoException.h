/*
 * MongoException.h
 *
 *  Created on: Apr 2, 2018
 *	  Author: tgburrin
 */

#ifndef MONGO_MONGOEXCEPTION_H_
#define MONGO_MONGOEXCEPTION_H_

#include <string>
#include <exception>

using namespace std;

class MongoException : exception {
private:
	string message;

public:
	MongoException(string);
	MongoException(const char *);
	virtual ~MongoException() throw();
	virtual const char* what();
};

#endif /* MONGO_MONGOEXCEPTION_H_ */
