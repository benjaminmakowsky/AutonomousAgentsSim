/*
 * Wrapper for sqlite3 objects and functions
 *
 */

#ifndef _SQLITE_WRAPPER_H
#define _SQLITE_WRAPPER_H

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

/*
 * DB_LOG_DELTA sets how often that we log data in units of seconds.
 * DB_CLOSE_TIMEOUT sets how many seconds we wait before logging an error.
 * e.g. - `#define DB_LOG_DELTA 1' means we log every second.
 */

#define DB_LOG_DELTA 1
#define DB_CLOSE_TIMEOUT 10
#define DB_DEFAULT_FILE "demo.db"

#define DB_COLUMN_TYPES "CREATE TABLE %s (kills SMALLINT, deaths SMALLINT, \
    float_dir FLOAT, last_wall_touch INT, survival_time FLOAT, dir INT, \
    name CHARACTER(80), username CHARACTER(80), visibility BOOLEAN, \
    damaged FLOAT, armor INT, cloak INT, shapename CHARACTER(80), \
    sight_range INT, fov FLOAT, privs INT);"

#define DB_COLUMNS (kills, deaths, float_dir, last_wall_touch, survival_time, \
    dir, name, username, visibility, damaged, armor, cloak, shapename, \
    sight_range, fov, privs)

#define DB_COLUMN_ARG pl->kills, pl->deaths, pl->float_dir, \
    pl->last_wall_touch, pl->survival_time, pl->dir, pl->name, \
    pl->username, pl->visibility->canSee, pl->damaged, pl->armor, \
    pl->cloak, pl->shapename, pl->sight_range, pl->fov, pl->privs

#define DB_INSERT "INSERT INTO %s (kills, deaths, float_dir, last_wall_touch, \
    survival_time, dir, name, username, visibility, damaged, armor, cloak, \
    shapename, sight_range, fov, privs ) values (%d, %d, %.4f, %d, %.4f, \
    %d, '%s', '%s', %d, %.4f, %d, %d, '%s', %d, %.4f, %d);"

struct dbStruct
{
    _Bool isInit;            // has db been initialized?
    sqlite3 *db;             // database connection object
    sqlite3_stmt *stmt;      // prepared statement object
    char *path;              // db path
    char *filename;          // db filename
    unsigned long timestamp; // time at db table creation
    char* tablename;         // tablename
    char* query;             // query string
    unsigned long lastLog;   // last time we completed a write to db
};

static struct dbStruct db = {.isInit = 0, .db = NULL, .stmt = NULL, \
                             .path = NULL, .filename = NULL, .timestamp = 0, \
                             .query = NULL, .lastLog = 0};

/*
 * wrapper prototypes
 */

void dbLog();
_Bool dbActiveLogWindow();
void dbTrxnStart();
void dbTrxnEnd();
void dbClose();

#endif
