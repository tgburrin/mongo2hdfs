# mongo2hdfs
Mongo to HDFS writer
This is a hack week project where the goal is to create a process that tails the Mongo oplog on a cluster-wide basis and dumps the resulting data to HDFS.  Since this is a hack project, the implementation has a large number of known deficiencies just a few of which include:
* Updates will provide the most recent document - not the document calculated by the change
* Management of mutex locking is not well organized and thus it probably locks more frequently or for longer than it should.
* Failover - failover on the mongo or hdfs side has not been tested.
* syncing flushes with bookmark updates should probably be done in the same method or at least in the same object.
* flushes happen every 1000 records, but there should probably be a thread that does it by time (every 1k records OR 10s, whichever comes first).
* It's been a while since valgrind has been used to check it, so there might be leaks.

This currently links to [libjsoncpp](https://github.com/open-source-parsers/jsoncpp.git), [libbson-1.0 and libmongoc-1.0](https://github.com/mongodb/mongo-c-driver.git), [libhdfs3](https://github.com/Pivotal-Data-Attic/attic-c-hdfs-client.git), and [sqlite3](https://www.sqlite.org/download.html)

In the case of libjsoncpp one will likely need to set pass BUILD_SHARED_LIBS=on to cmake to be able to link properly to it.
