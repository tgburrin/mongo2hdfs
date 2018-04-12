/*
 * HdfsFileFactory.cpp
 *
 *  Created on: Apr 10, 2018
 *      Author: tburrington
 */

#include "HdfsFileFactory.h"

HdfsFileFactory::HdfsFileFactory(ProcessCfg *cfg) :
	username(cfg->getHdfsUsername()),
	hostname(cfg->getHdfsNameNode()),
	basePath(cfg->getHdfsBasePath()),
	port(cfg->getHdfsPort()),
	replicationFactor(cfg->getHdfsReplicationFactor())
{}

HdfsFileFactory::~HdfsFileFactory() {}

void HdfsFileFactory::setPort(uint32_t p) {
	port = p;
}

HdfsFile *HdfsFileFactory::getFile(string fn) {
	HdfsFile *f = new HdfsFile(username, hostname, port, replicationFactor, basePath);
	f->openFile(fn);
	return f;
}
