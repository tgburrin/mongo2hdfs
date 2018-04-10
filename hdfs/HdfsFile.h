/*
 * HdfsFile.h
 *
 *  Created on: Apr 7, 2018
 *	  Author: tgburrin
 */

#ifndef HDFS_HDFSFILE_H_
#define HDFS_HDFSFILE_H_

#include <iostream>
#include <cstdint>
#include <thread>
#include <mutex>

#include <hdfs/hdfs.h>

#include "HdfsFileException.h"
#include "HdfsFileFactory.h"

using namespace std;

class HdfsFile {
friend class HdfsFileFactory;

private:
	string username;
	string hostname;
	uint32_t port = 8020;

	string basePath;
	string fileName;

	struct hdfsBuilder *hdfsCfg = NULL;
	hdfsFS fileSystem = NULL;
	hdfsFile fileDescriptor = NULL;

	uint32_t batchCounter = 0;

	bool closeFile();
	bool flushFile();
	bool openFile(string);
	bool changeFile(string);
	void init();

	HdfsFile(string, string, uint32_t, string);

public:
	mutex *lck = NULL;
	bool writeToFile(string);
	// We might want to make this private and have a cleaning process
	virtual ~HdfsFile();
};

#endif /* HDFS_HDFSFILE_H_ */
