/*
 * HdfsFileFactory.h
 *
 *  Created on: Apr 10, 2018
 *      Author: tburrington
 */

#ifndef HDFS_HDFSFILEFACTORY_H_
#define HDFS_HDFSFILEFACTORY_H_

#include "HdfsFile.h"

using namespace std;

class HdfsFile;
class HdfsFileFactory {
	string username;
	string hostname;
	string basePath = "/";

	uint32_t port = 8020;

public:
	HdfsFileFactory(string, string, string);
	virtual ~HdfsFileFactory();

	void setPort(uint32_t);
	HdfsFile *getFile(string);
};

#endif /* HDFS_HDFSFILEFACTORY_H_ */
