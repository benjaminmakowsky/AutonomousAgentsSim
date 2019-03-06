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
 *     * move open, close, etc function to server.c:main()
 *     * move -lsqlite3, -lpthread to a better place in build scripts
 *     * add setters/getters for directory
 */

static _Bool dbOpen()
{
    if (db.isInit == 1)
        return db.isInit;

    // open database
    sqlite3_open(DB_DEFAULT_FILE, &db.db);

    if (db.db == NULL)
    {
        warn("sqlite_wrapper: dbOpen() open: %s\n", sqlite3_errmsg(db.db));
        return 0;
    }

    /*
     *  Naming the databases based on their timestamp is OK for now, but lets think 
     *  about how we can categorize these in the future
     */

    // set timestamp and name for the table
    asprintf(&db.timestamp, "%lu", (unsigned long) time(NULL));
    asprintf(&db.tablename, "db_%s", db.timestamp);

    // create table for this run
    asprintf(&db.query, DB_COLUMN_TYPES, db.tablename);

    if (sqlite3_prepare_v2(db.db, db.query, strlen(db.query), &db.stmt, NULL) != SQLITE_OK) {
        warn("sqlite_wrapper: dbOpen() prepare: %s\n", sqlite3_errmsg(db.db));
        return 0;
    } else if (sqlite3_step(db.stmt) != SQLITE_DONE) {
        warn("sqlite_wrapper: dbOpen() step: %s\n", sqlite3_errmsg(db.db));
        return 0;
    } else {
        db.isInit = 1;
        xpinfo("sqlite_wrapper: dbOpen() successful\n");
    }

    sqlite3_finalize(db.stmt);
    free(db.query);

    return 1;
} 

void dbClose()
{
    if (db.isInit == 0)
        return;

    /*
     * Make sure we have written everything to file before killing server
     */
    int i;
    for (i = 0; sqlite3_close(db.db) != SQLITE_OK; i++) {
        if (i > DB_CLOSE_TIMEOUT) {
            warn("sqlite_wrapper: dbClose() failed to close after \
                  DB_CLOSE_TIMEOUT seconds: %s\n", sqlite3_errmsg(db.db));
            return;
        }
    }

    xpinfo("sqlite_wrapper: dbClose() successful\n");

    free(db.tablename);
}

void dbTrxnStart()
{
    sqlite3_exec(db.db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
}

void dbTrxnEnd() 
{
    sqlite3_exec(db.db, "END TRANSACTION;", NULL, NULL, NULL);
}

/*
 * logging implementation:
 *     To avoid refactoring the implementation whenever we want to log something
 *     different or change the schema entirely, it may make sense to just
 *     pass all the core data structures, then have the logging function include
 *     a parameter which is of type "macro enum" which changes the behavior.
 *     The macro may possibly expand to a corresponding prepare statement in 
 *     the form of a constant expression
 */

void dbLog(player_t *pl)
{
    /*
     * Generally for logging we wouldn't want to halt the server if there
     * is a problem with capturing logs, but in this case we may want to as 
     * the current goal is to caputure that data, and it would not be good
     * to realize logging wasn't working during a simulation job.
     */

    if (dbOpen() == 0)
        return;

    asprintf(&db.query, DB_INSERT, db.tablename, DB_COLUMN_ARG);

    if (sqlite3_prepare_v2(db.db, db.query, strlen(db.query), &db.stmt, NULL) != SQLITE_OK) {
        warn("sqlite_wrapper: dbLog() prepare: %s\n", sqlite3_errmsg(db.db));
        return;
    }

    if (sqlite3_step(db.stmt) != SQLITE_DONE) {
        warn("sqlite_wrapper: dbLog() step: %s\n", sqlite3_errmsg(db.db));
        return;
    }

    if (sqlite3_finalize(db.stmt) != SQLITE_OK)
        warn("sqlite_wrapper: dbLog() finalize: %s\n", sqlite3_errmsg(db.db));

    free(db.query);

    return;
}

_Bool dbActiveLogWindow()
{
    unsigned long currtime = (unsigned long) time(NULL);

    /*
     * We want to initiate logging when two conditions are met:
     * (1) - Current time is different from when we last logged,
     * (2) - DB_LOG_DELTA seconds have elapsed since we last logged. 
     *
     * Since we call our logging function in `update()' which runs on each frame
     * (read: many times per second) we require condition (1) to make sure we 
     * don't log multiple times during a single second.
     */

    if (currtime != db.lastLog && currtime >= db.lastLog + DB_LOG_DELTA)
    {
        db.lastLog = currtime;
        return 1;
    }

    return 0;
}
