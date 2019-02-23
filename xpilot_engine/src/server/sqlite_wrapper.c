#include <sys/types.h>
#include <sys/stat.h>

#include <pthread.h>
#include <time.h>
#include <string.h>
#include <sqlite3.h>
#include "xpserver.h"

#include "sqlite_wrapper.h"
#include "player.h"

/* TODO: 
 * 	* move open, close, etc function to server.c:main()
 * 	* move -lsqlite3, -lpthread to a better place in build scripts
 * 	* add setters/getters for directory
 */

static _Bool db_open()
{

	if (db.isInit == 1)
		return db.isInit;

	// open database
	sqlite3_open(DB_DEFAULT_FILE, &db.db);

	if (db.db == NULL)
	{
		warn("sqlite_wrapper: sqlite3_open() failed\n");
		return 0;
	}

	xpinfo("sqlite_wrapper: sqlite3_open() successful\n");

	// set timestamp and name for the table
	asprintf(&db.timestamp, "%lu", (unsigned long) time(NULL));
	asprintf(&db.tablename, "db_%s", db.timestamp);

	// create table for this run
	asprintf(&db.query, DB_COLUMN_TYPES, db.tablename);
	sqlite3_prepare_v2(db.db, db.query, strlen(db.query), &db.stmt, NULL);

	if (sqlite3_step(db.stmt) != SQLITE_DONE) {
		warn("sqlite_wrapper: db_open() failed: %s\n", sqlite3_errmsg(db.db));
		return 0;
	}

	xpinfo("sqlite_wrapper: db_open() successful\n");

	free(db.query);

	return db.isInit = 1;
} 

static void db_create()
{
}

void db_close()
{
	if (db.isInit == 0)
		return;

	sqlite3_finalize(db.stmt);
	sqlite3_close(db.db);

	xpinfo("sqlite_wrapper: db_close() successful\n");

	free(db.tablename);
	//free(db.path);
	//free(db.filename);
}

void db_txnStart()
{
	sqlite3_exec(db.db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
}

void db_txnEnd() 
{
	sqlite3_exec(db.db, "END TRANSACTION;", NULL, NULL, NULL);
}

/*
 * logging implementation:
 * 	To avoid refactoring the implementation whenever we want to log something
 * 	different or change the schema entirely, it may make sense to just
 * 	pass all the core data structures, then have the logging function include
 * 	a parameter which is of type "macro enum" which changes the behavior.
 * 	The macro may possibly expand to a corresponding prepare statement in 
 * 	the form of a constant expression
 */

void db_log(player_t *pl)
{
	/*
	 * Generally for logging we wouldn't want to halt the server if there
	 * is a problem with capturing logs, but in this case we may want to as 
	 * the current goal is to caputure that data, and it would not be good
	 * to realize logging wasn't working during a simulation job.
	 */

	if (db_open() == 0)
		return;

	//asprintf(&db.query, "insert into %s (name, dir) values ('%s', %d);", db.tablename, pl->name, pl->dir);
	//asprintf(&db.query, DB_TERRIBLE, db.tablename, DB_COLUMN_ARG);

	asprintf(&db.query, DB_INSERT, db.tablename, DB_COLUMN_ARG);
	sqlite3_prepare_v2(db.db, db.query, strlen(db.query), &db.stmt, NULL);

	if (sqlite3_step(db.stmt) != SQLITE_DONE) {
		warn("ERROR sqlite_wrapper.c inserting data: %s", sqlite3_errmsg(db.db));
		return;
	}

	free(db.query);

	return;
}

_Bool db_quantum()
{
	unsigned long currtime = (unsigned long) time(NULL);

	if (currtime ^ db.lastLog != 0 && currtime >= db.lastLog + DB_FREQ)
	{
		db.lastLog = currtime;
		return 1;
	}
	return 0;
}

static char *db_setFilename(char *filename)
{
	// malloc 
	//db.path     = malloc(1024 * sizeof(char)); 
	//db.filename = malloc(128 * sizeof(char));

	// TODO: Expose filename and path setters instead of hardcoding defaults
	//strcpy(db.path, DB_DEFAULT_PATH);
	//strcpy(db.filename, DB_DEFAULT_FILENAME);
	//strcat(db.filename, DB_DEFAULT_EXT);

}

static char *db_setPath(char *path)
{
	/*
	   struct stat st;

	   if (stat(arg, &st) == 0) 
	   {
	   if (S_ISDIR(st.st_mode))
	   {
	   strcpy(db.path, arg);
	   } 
	   else if (S_ISREG(st.st_mode))
	   { 
	   strcpy(db.filename, arg);
	   }
	   }
	   */
}

