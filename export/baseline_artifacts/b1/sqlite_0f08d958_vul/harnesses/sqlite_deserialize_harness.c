/*
 * B1 Human-written harness: SQLite deserialize path
 * Targets: sqlite3_deserialize — parsing untrusted database blobs
 */
#include "sqlite3.h"
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

#define MAX_DB_SIZE 512

int main(void) {
    unsigned char db_blob[MAX_DB_SIZE];
    klee_make_symbolic(db_blob, sizeof(db_blob), "db_blob");

    /* Require valid SQLite header magic */
    memcpy(db_blob, "SQLite format 3\000", 16);

    sqlite3 *db = NULL;
    int rc = sqlite3_open(":memory:", &db);
    if (rc != SQLITE_OK || db == NULL) {
        sqlite3_close(db);
        return 0;
    }

    /* Allocate a copy for sqlite3_deserialize (it takes ownership) */
    unsigned char *buf = (unsigned char *)sqlite3_malloc64(MAX_DB_SIZE);
    if (!buf) {
        sqlite3_close(db);
        return 0;
    }
    memcpy(buf, db_blob, MAX_DB_SIZE);

    rc = sqlite3_deserialize(db, "main", buf, MAX_DB_SIZE, MAX_DB_SIZE,
                             SQLITE_DESERIALIZE_FREEONCLOSE |
                             SQLITE_DESERIALIZE_RESIZEABLE);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return 0;
    }

    /* Try to query the deserialized database */
    sqlite3_stmt *stmt = NULL;
    rc = sqlite3_prepare_v2(db, "SELECT * FROM sqlite_master;", -1, &stmt, NULL);
    if (rc == SQLITE_OK && stmt) {
        for (int i = 0; i < 5; i++) {
            if (sqlite3_step(stmt) != SQLITE_ROW) break;
        }
        sqlite3_finalize(stmt);
    }

    sqlite3_close(db);
    return 0;
}
