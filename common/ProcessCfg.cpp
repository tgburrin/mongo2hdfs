/*
 * ProcessCfg.cpp
 *
 *  Created on: Oct 10, 2017
 *	  Author: tgburrin
 */

#include "ProcessCfg.h"

ProcessCfg::ProcessCfg(string cfgFile) {
	_ParseConfig(cfgFile);
}

ProcessCfg::~ProcessCfg() {
}

void ProcessCfg::_ParseConfig (string config_file) {
	Json::Value doc;
	Json::CharReaderBuilder crb;

	string errors;

	ifstream infile(config_file.c_str());

	if ( !Json::parseFromStream(crb, infile, &doc, &errors) )
		throw ApplicationException("Could not parse config file "+config_file+": "+errors);

	infile.close();

	vector<string> required = {"MongosURI", "HDFSNameNode", "HDFSUsername", "HDFSPath", "StatePath"};
	for ( auto req : required )
		if ( doc[req].isNull() || doc[req].asString().empty())
			throw ApplicationException(req+" must be provided in the configuration");

	mongosURI = doc["MongosURI"].asString();

	initializeFromStart = false;
	if ( !doc["MongoInitializeOplogStart"].isNull() )
		initializeFromStart = doc["MongoInitializeOplogStart"].asBool();

	hdfsNameNode = doc["HDFSNameNode"].asString();
	hdfsUsername = doc["HDFSUsername"].asString();
	hdfsBasePath = doc["HDFSPath"].asString();

	if ( !doc["HDFSPort"].isNull() )
		hdfsPort = doc["HDFSPort"].asUInt();

	if ( !doc["HDFSReplicationFactor"].isNull() )
		hdfsReplicationFactor = doc["HDFSReplicationFactor"].asUInt();

	statePath = doc["StatePath"].asString();
}

bool ProcessCfg::DebugEnabled() {
	return Debug;
}

string ProcessCfg::getMongosURI() {
	return mongosURI;
}

bool ProcessCfg::getMongoInitFromStart() {
	return initializeFromStart;
}

string ProcessCfg::getHdfsNameNode() {
	return hdfsNameNode;
}

string ProcessCfg::getHdfsUsername() {
	return hdfsUsername;
}

string ProcessCfg::getHdfsBasePath() {
	return hdfsBasePath;
}

uint32_t ProcessCfg::getHdfsPort() {
	return hdfsPort;
}

uint32_t ProcessCfg::getHdfsReplicationFactor() {
	return hdfsReplicationFactor;
}

string ProcessCfg::getStatePath() {
	return statePath;
}
