/*
 * HdfsFile.cpp
 *
 *  Created on: Apr 7, 2018
 *	  Author: tgburrin
 */

#include "HdfsFile.h"

HdfsFile::HdfsFile(string u, string h, uint32_t p, uint32_t rf, string bp) : username(u), hostname(h), port(p), replicationFactor(rf), basePath(bp) {
	batchCounter = 0;

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

	rv = false;
	batchCounter++;
	if ( batchCounter >= 1000 ) {
		flushFile();
		rv = true;
		batchCounter = 0;
	}
	return rv;
}

bool HdfsFile::changeFile(string newFileName) {
	closeFile();
	return openFile(newFileName);
}

bool HdfsFile::closeFile() {
	bool rv = false;

	if ( fileSystem != NULL && fileDescriptor != NULL ) {
		cout << "Flushing " << fileName << endl << flush;
		flushFile();
		rv = hdfsCloseFile(fileSystem, fileDescriptor) == 0 ? true : false;
	}

	return rv;
}

bool HdfsFile::flushFile() {
	return hdfsFlush(fileSystem, fileDescriptor) == 0 ? true : false;
}
