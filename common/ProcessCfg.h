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
	string hdfsNameNode;
	string hdfsUsername;
	string hdfsBasePath;
	uint32_t hdfsPort = 0;

	string statePath;

public:
	ProcessCfg(string);
	virtual ~ProcessCfg();

	bool DebugEnabled();

	string getMongosURI();
	string getHdfsNameNode();
	string getHdfsUsername();
	string getHdfsBasePath();
	uint32_t getHdfsPort();
	string getStatePath();
};

#endif /* PROCESSCFG_H_ */
