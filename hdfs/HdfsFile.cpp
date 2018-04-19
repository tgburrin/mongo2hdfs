/*
 * HdfsFile.cpp
 *
 *  Created on: Apr 7, 2018
 *	  Author: tgburrin
 */

#include "HdfsFile.h"

HdfsFile::HdfsFile(string u, string h, uint32_t p, uint32_t rf, string bp) : username(u), hostname(h), port(p), replicationFactor(rf), basePath(bp) {
	unflushedWrites = 0;

	if ( lck == NULL )
		lck = new mutex();

	if ( hdfsCfg == NULL ) {
		hdfsCfg = hdfsNewBuilder();
		hdfsBuilderSetUserName(hdfsCfg, username.c_str());
		hdfsBuilderSetNameNode(hdfsCfg, hostname.c_str());
		hdfsBuilderSetNameNodePort(hdfsCfg, port);

		fileSystem = hdfsBuilderConnect(hdfsCfg);

		if ( fileSystem == NULL )
			throw HdfsFileException("Unable to create hdfs filesystem: "+string(hdfsGetLastError()));
	}
}

HdfsFile::~HdfsFile() {
	closeFile();

	if ( fileSystem != NULL )
		hdfsDisconnect(fileSystem);


	if ( hdfsCfg != NULL )
		hdfsFreeBuilder(hdfsCfg);

	delete lck;
}

bool HdfsFile::openFile(string fn) {
	if ( fileDescriptor != NULL )
		return true;

	fileName = fn;

	if ( fileSystem == NULL )
		throw HdfsFileException("Filesystem handle has not been initialized");

	// Read/write + append, default buffer size, replication factor (1 for testing), default block size
	fileDescriptor = hdfsOpenFile(fileSystem, (basePath + "/" + fileName).c_str(), O_WRONLY|O_APPEND, 0, replicationFactor, 0);
	if ( fileDescriptor == NULL )
		throw HdfsFileException("Unable to initialize the file descriptor: "+string(hdfsGetLastError()));

	return true;
}
bool HdfsFile::writeToFile(string message) {
	bool rv = hdfsWrite(fileSystem, fileDescriptor, (message + "\n").c_str(), message.length()+1) >= 0 ? true : false;
	if ( !rv )
		throw HdfsFileException("Write to "+fileName+" failed");

	unflushedWrites++;
	return rv;
}

bool HdfsFile::changeFile(string newFileName) {
	closeFile();
	return openFile(newFileName);
}

bool HdfsFile::closeFile() {
	if ( fileSystem != NULL && fileDescriptor != NULL )
		return hdfsCloseFile(fileSystem, fileDescriptor) == 0 ? true : false;
	return true;
}

bool HdfsFile::flushFile() {
	if ( unflushedWrites > 0 ) {
		if ( hdfsFlush(fileSystem, fileDescriptor) == 0 )
			unflushedWrites = 0;
		else
			throw HdfsFileException("Unable to flush "+fileName+": "+string(hdfsGetLastError()));
	}
	return true;
}
