/*
 * BookmarkManager.h
 *
 *  Created on: Apr 10, 2018
 *      Author: tburrington
 */

#ifndef COMMON_BOOKMARKMANAGER_H_
#define COMMON_BOOKMARKMANAGER_H_

#include <iostream>
#include <mutex>

#include <bson.h>
#include <sqlite3.h>

#include "ApplicationException.h"

using namespace std;

class BookmarkManager {
private:
	sqlite3 *db = NULL;
	mutex *lck = NULL;

public:
	BookmarkManager(string);
	virtual ~BookmarkManager();
	void setBookmark(string, uint32_t, uint32_t);
	bson_t *getTimestampBookmark(string);
	bool getTimestampBookmarkValues(string, uint32_t *, uint32_t *);
};

#endif /* COMMON_BOOKMARKMANAGER_H_ */
