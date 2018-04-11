/*
 * BookmarkManager.cpp
 *
 *  Created on: Apr 10, 2018
 *      Author: tburrington
 */

#include "BookmarkManager.h"

BookmarkManager::BookmarkManager(string dbPath) {
	sqlite3_stmt *stmt;

	if ( sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK )
		throw ApplicationException("Unable to open db "+dbPath+": "+sqlite3_errmsg(db));

	sqlite3_prepare_v2(db, "select name from sqlite_master WHERE type='table' and name='mongo_bookmarks'", -1, &stmt, NULL);

	uint32_t numTables = 0;
	while (sqlite3_step(stmt) != SQLITE_DONE)
		numTables++;

	sqlite3_finalize(stmt);
	stmt = NULL;

	if ( numTables > 1 ) {
		throw ApplicationException("Too many rows returned while checking for mongo_bookmarks table");
	} else if ( numTables == 0 ) {
		string cmd = "create table mongo_bookmarks ( shard_id text not null primary key, epoch_time int not null, txn_offset int not null )";
		char *errmsg = NULL;
		if ( sqlite3_exec(db, cmd.c_str(), 0, 0, &errmsg) != SQLITE_OK )
			throw ApplicationException("Could not create mongo_bookmarks table");
	}
}

BookmarkManager::~BookmarkManager() {
	sqlite3_close(db);
}

bool BookmarkManager::getTimestampBookmarkValues(string shardName, uint32_t *timestamp, uint32_t *increment) {
	bool rv = false;
	sqlite3_stmt *stmt;
	uint32_t rows = 0;

	string query = "select epoch_time, txn_offset from sqlite_master where shard_id = '"+shardName+"'";
	sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL);

	while(sqlite3_step(stmt) != SQLITE_DONE) {
		rv = true;
		*timestamp = sqlite3_column_int(stmt, 0);
		*increment = sqlite3_column_int(stmt, 1);
	}

	sqlite3_finalize(stmt);
	return rv;
}