#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
   Self-contained reproducer for the out-of-bounds read caused by calling
   isdigit() with a potentially negative char value (undefined behavior).

   We replicate the vulnerable helper from contrib/gregbook/rpng2-x.c:
       static int is_number(char *p) {
           while (*p) {
               if (!isdigit(*p))
                   return FALSE;
               ++p;
           }
           return TRUE;
       }

   To make the UB observable under ASan on any libc, we provide a stub
   implementation of isdigit() that mimics a table-based ctype and indexes
   a too-small table directly by the int argument (without casting to
   unsigned char). Passing a byte with value 0x80 (which may be negative on
   platforms where char is signed) will index out of bounds and trigger ASan.
*/

#define TRUE 1
#define FALSE 0

/* Intentionally tiny classification table to force OOB on bad inputs. */
static const unsigned char tiny_ctype_digit_table[16] = {
    /* Only small indices are valid; everything else will be OOB. */
    0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
};

/* Stub isdigit with the same signature, intentionally vulnerable: no bounds check,
   no cast to unsigned char; indexes directly by c. */
static int isdigit(int c)
{
    /* This line will perform an out-of-bounds read when c is negative (e.g., -128)
       or larger than 15 (e.g., 128), which is exactly the UB the real ctype may
       exhibit when passed a signed char outside the unsigned char range. */
    return tiny_ctype_digit_table[c] != 0;
}

/* Vulnerable helper, copied to mirror the project bug (no cast to unsigned char). */
static int is_number(char *p)
{
    while (*p) {
        if (!isdigit(*p))
            return FALSE;
        ++p;
    }
    return TRUE;
}

int main(void)
{
    /* Craft input with a single byte 0x80 followed by NUL terminator. On platforms
       where char is signed (common on x86), *p will be -128. Even if char is
       unsigned, *p will be 128; both cause OOB access with the tiny table above. */
    char s[2];
    s[0] = (char)0x80; /* problematic byte (may be negative if char is signed) */
    s[1] = '\0';

    /* Call the vulnerable function: this will call the stub isdigit with c = *p
       and trigger an out-of-bounds read flagged by ASan. */
    int res = is_number(s);

    /* Print to keep side effects visible (though ASan should report before). */
    printf("is_number returned: %d\n", res);
    return 0;
}
