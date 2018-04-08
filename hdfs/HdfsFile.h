/*
 * HdfsFile.h
 *
 *  Created on: Apr 7, 2018
 *      Author: tgburrin
 */

#ifndef HDFS_HDFSFILE_H_
#define HDFS_HDFSFILE_H_

#include <iostream>
#include <cstdint>
#include <thread>
#include <mutex>

#include <hdfs/hdfs.h>
#include "HdfsFileException.h"

using namespace std;

class HdfsFile {
private:
	string username;
	string hostname;
	uint32_t port = 8020;

	string basePath;
	string fileName;

	struct hdfsBuilder *hdfsCfg = NULL;
	hdfsFS fileSystem = NULL;
	hdfsFile fileDescriptor = NULL;

	void init();
	bool closeFile();
	bool flushFile();

public:
	mutex *lck = NULL;

	HdfsFile(string, string);
	HdfsFile(string, string, uint32_t);

	void setBasePath(string);
	string getBasePath();

	bool openFile(string);
	bool writeToFile(string);

	bool changeFile(string);

	virtual ~HdfsFile();
};

#endif /* HDFS_HDFSFILE_H_ */
