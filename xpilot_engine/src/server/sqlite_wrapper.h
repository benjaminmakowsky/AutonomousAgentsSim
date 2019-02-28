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
 * #defines
 */

/*
  Get rid of these commented defined
*/

//#define DB_DEFAULT_EXT ".db"
//#define DB_DEFAULT_FILENAME "demo"
//#define DB_DEFAULT_PATH ""
#define DB_DEFAULT_FILE "demo.db"
#define DB_FREQ 1

/*
 * Get rid of your notes here
 */

// Default location is ./xpilot-le/xpilot_bin/server/demo.db
// include boids params (+ enemy avoidance)-> eval w/ weight for individual bots
// boids + pathfinding + obstacle course (baby steps this)
// count unit wall/collision (perhaps different weight)

/*
 * I see you are using a column limit of 80 characters. Nice!
 */

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

/*
 *  Change "insert into" to Uppercase, to stick with the style 
 */

#define DB_INSERT "INSERT INTO %s (kills, deaths, float_dir, last_wall_touch, \
	survival_time, dir, name, username, visibility, damaged, armor, cloak, \
	shapename, sight_range, fov, privs ) values (%d, %d, %.4f, %d, %.4f, \
	%d, '%s', '%s', %d, %.4f, %d, %d, '%s', %d, %.4f, %d);"

/*
 *  I see that you are trying to the function naming conventions
 *  of the XPilot code, but those conventions are old and outdated.
 *  Lets use 'modern' naming conventions for this, i.e:
 *  dbLog() instead of db_log()
 *  dbStruct insted of db_struct
 */

/*
 * Fixed the indentation in the stmt comment
 */

struct db_struct
{
	_Bool isInit;            // has db been initialized?
	sqlite3 *db; 	           // database connection object
	sqlite3_stmt *stmt;      // prepared statement object
	char *path;              // db path
	char *filename;          // db filename
	unsigned long timestamp; // time at db table creation
	char* tablename;         // tablename
	char* query;             // query string
	unsigned long lastLog;   // last time we completed a write to db
};

/*
 *  
 */

static struct db_struct db = {.isInit = 0, .db = NULL, .stmt = NULL, \
			      .path = NULL, .filename = NULL, .timestamp = 0, \
			      .query = NULL, .lastLog = 0};

/*
 * wrapper prototypes
 */

/*
 *  What is 'txn' short for? Transaction? Maybe use a more obvious
 *  short-hand name.
*/

void db_log();
_Bool db_quantum();
void db_txnStart();
void db_txnEnd();
void db_close();

#endif
