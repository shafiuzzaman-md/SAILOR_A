/*
 * B1 Human-written harness: SQLite prepared statement path
 * Targets: sqlite3_prepare_v2, sqlite3_bind_*, sqlite3_step, sqlite3_finalize
 */
#include "sqlite3.h"
#include <stdlib.h>
#include <string.h>
#include <klee/klee.h>

#define MAX_SQL_LEN 128
#define MAX_BIND_LEN 32

int main(void) {
    char sql[MAX_SQL_LEN];
    char bind_text[MAX_BIND_LEN];
    int bind_int;
    double bind_double;
    unsigned char bind_blob[MAX_BIND_LEN];

    klee_make_symbolic(sql, sizeof(sql), "sql_input");
    klee_make_symbolic(bind_text, sizeof(bind_text), "bind_text");
    klee_make_symbolic(&bind_int, sizeof(bind_int), "bind_int");
    klee_make_symbolic(&bind_double, sizeof(bind_double), "bind_double");
    klee_make_symbolic(bind_blob, sizeof(bind_blob), "bind_blob");

    sql[MAX_SQL_LEN - 1] = '\0';
    bind_text[MAX_BIND_LEN - 1] = '\0';

    sqlite3 *db = NULL;
    int rc = sqlite3_open(":memory:", &db);
    if (rc != SQLITE_OK || db == NULL) {
        sqlite3_close(db);
        return 0;
    }

    sqlite3_exec(db, "CREATE TABLE t1(a INTEGER, b TEXT, c REAL, d BLOB);",
                 NULL, NULL, NULL);

    sqlite3_stmt *stmt = NULL;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK || stmt == NULL) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 0;
    }

    /* Bind symbolic parameters */
    int nparams = sqlite3_bind_parameter_count(stmt);
    if (nparams > 0) {
        sqlite3_bind_int(stmt, 1, bind_int);
    }
    if (nparams > 1) {
        sqlite3_bind_text(stmt, 2, bind_text, -1, SQLITE_TRANSIENT);
    }
    if (nparams > 2) {
        sqlite3_bind_double(stmt, 3, bind_double);
    }
    if (nparams > 3) {
        sqlite3_bind_blob(stmt, 4, bind_blob, sizeof(bind_blob), SQLITE_TRANSIENT);
    }

    /* Step through results */
    for (int i = 0; i < 10; i++) {
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_ROW) break;

        /* Read columns */
        int ncols = sqlite3_column_count(stmt);
        for (int j = 0; j < ncols && j < 4; j++) {
            int type = sqlite3_column_type(stmt, j);
            switch (type) {
                case SQLITE_INTEGER:
                    (void)sqlite3_column_int64(stmt, j);
                    break;
                case SQLITE_FLOAT:
                    (void)sqlite3_column_double(stmt, j);
                    break;
                case SQLITE_TEXT:
                    (void)sqlite3_column_text(stmt, j);
                    break;
                case SQLITE_BLOB:
                    (void)sqlite3_column_blob(stmt, j);
                    (void)sqlite3_column_bytes(stmt, j);
                    break;
            }
        }
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}
