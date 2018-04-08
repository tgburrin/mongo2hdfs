/*
 * HdfsFileException.cpp
 *
 *  Created on: Apr 7, 2018
 *      Author: tgburrin
 */

#include "HdfsFileException.h"

HdfsFileException::HdfsFileException(string m) : message(m) {}
HdfsFileException::HdfsFileException(const char *m) : message(m) {}
HdfsFileException::~HdfsFileException() {}

const char* HdfsFileException::what()
{
	return message.c_str();
}
