/*
 * HdfsFileFactory.cpp
 *
 *  Created on: Apr 10, 2018
 *      Author: tburrington
 */

#include "HdfsFileFactory.h"

HdfsFileFactory::HdfsFileFactory(string u, string h, string bp) : username(u), hostname(h), basePath (bp) {}
HdfsFileFactory::~HdfsFileFactory() {}

void HdfsFileFactory::setPort(uint32_t p) {
	port = p;
}

HdfsFile *HdfsFileFactory::getFile(string fn) {
	HdfsFile *f = new HdfsFile(username, hostname, port, basePath);
	f->openFile(fn);
	return f;
}
