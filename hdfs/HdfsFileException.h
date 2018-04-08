/*
 * HdfsFileException.h
 *
 *  Created on: Apr 7, 2018
 *	  Author: tgburrin
 */

#ifndef HDFS_HDFSFILEEXCEPTION_H_
#define HDFS_HDFSFILEEXCEPTION_H_

#include <iostream>
#include <exception>

using namespace std;

class HdfsFileException: exception {
private:
	string message;

public:
	HdfsFileException(string);
	HdfsFileException(const char *);
	virtual ~HdfsFileException();
	virtual const char* what();
};

#endif /* HDFS_HDFSFILEEXCEPTION_H_ */
