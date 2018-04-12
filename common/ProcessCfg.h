/*
 * ProcessCfg.h
 *
 *  Created on: Oct 10, 2017
 *	  Author: tgburrin
 */

#ifndef PROCESSCFG_H_
#define PROCESSCFG_H_

#include <iostream>
#include <fstream>
#include <vector>

#include <json/json.h>
#include <json/reader.h>

#include "ApplicationException.h"

using namespace std;

class ProcessCfg {
private:
	bool Debug = false;
	void _ParseConfig(string);

	string mongosURI;
	bool initializeFromStart;

	string hdfsNameNode;
	string hdfsUsername;
	string hdfsBasePath;
	uint32_t hdfsPort = 8020;
	uint32_t hdfsReplicationFactor = 3;

	string statePath;

public:
	ProcessCfg(string);
	virtual ~ProcessCfg();

	bool DebugEnabled();

	string getMongosURI();
	bool getMongoInitFromStart();
	string getHdfsNameNode();
	string getHdfsUsername();
	string getHdfsBasePath();
	uint32_t getHdfsPort();
	uint32_t getHdfsReplicationFactor();
	string getStatePath();
};

#endif /* PROCESSCFG_H_ */
