#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Minimal stand-in for OpenSSL's ASN1_STRING */
typedef struct asn1_string_st {
    int length;
    int type;
    unsigned char *data;
    long flags;
} ASN1_STRING;

/* Vulnerable function from apps/lib/apps.c */
void corrupt_signature(const ASN1_STRING *signature)
{
    unsigned char *s = signature->data;
    /* BUG: No check that signature->length > 0 */
    s[signature->length - 1] ^= 0x1;  /* Out-of-bounds write if length == 0 */
}

int main(void)
{
    /* Allocate a 1-byte heap buffer so ASan can catch the write just before it */
    unsigned char *buf = (unsigned char *)malloc(1);
    if (buf == NULL) {
        perror("malloc");
        return 1;
    }
    buf[0] = 0xAA;

    /* Craft ASN1_STRING with length == 0 but non-NULL data pointer */
    ASN1_STRING sig;
    sig.length = 0;          /* Triggers s[-1] access in corrupt_signature */
    sig.data = buf;          /* Points to heap so ASan reports heap OOB write */
    sig.type = 0;
    sig.flags = 0;

    fprintf(stderr, "Calling corrupt_signature with length=0, data=%p\n", (void*)buf);
    corrupt_signature(&sig);

    /* If the program reaches here without ASan aborting, print something */
    fprintf(stderr, "Finished call (this should not happen under ASan). buf[0]=%02x\n", buf[0]);
    free(buf);
    return 0;
}
