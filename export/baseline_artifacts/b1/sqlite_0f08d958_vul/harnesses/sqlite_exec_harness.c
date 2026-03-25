/*
 * B1 Human-written harness: SQLite SQL execution path
 * Targets: sqlite3_open, sqlite3_exec, sqlite3_close
 * Modeled after OSS-Fuzz sqlite3_ossfuzz
 */
#include "sqlite3.h"
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

#define MAX_SQL_LEN 128

int main(void) {
    char sql[MAX_SQL_LEN];
    klee_make_symbolic(sql, sizeof(sql), "sql_input");
    sql[MAX_SQL_LEN - 1] = '\0'; /* ensure null termination */

    sqlite3 *db = NULL;
    int rc = sqlite3_open(":memory:", &db);
    if (rc != SQLITE_OK || db == NULL) {
        sqlite3_close(db);
        return 0;
    }

    /* Create a table so queries have something to work with */
    sqlite3_exec(db, "CREATE TABLE t1(a INTEGER, b TEXT, c REAL);", NULL, NULL, NULL);
    sqlite3_exec(db, "INSERT INTO t1 VALUES(1,'hello',3.14);", NULL, NULL, NULL);

    /* Execute symbolic SQL */
    char *errmsg = NULL;
    sqlite3_exec(db, sql, NULL, NULL, &errmsg);
    if (errmsg) sqlite3_free(errmsg);

    sqlite3_close(db);
    return 0;
}
