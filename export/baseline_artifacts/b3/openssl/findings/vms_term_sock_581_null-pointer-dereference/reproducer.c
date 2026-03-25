#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>

/* Stub for OpenSSL's BIO_snprintf used by the vulnerable function */
int BIO_snprintf(char *buf, size_t n, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int ret = vsnprintf(buf, n, format, ap);
    va_end(ap);
    return ret;
}

/* Interpose localtime() to force a failure and return NULL */
struct tm *localtime(const time_t *timer) {
    (void)timer;
    return NULL; /* Simulate localtime() failure */
}

/* Vulnerable function copied from the source context */
static void LogMessage(char *msg, ...) {
    char *Month[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    static unsigned int pid = 0;
    va_list args;
    time_t CurTime;
    struct tm *LocTime;
    char MsgBuff[256];

    /* Get the process pid */
    if (pid == 0)
        pid = getpid();

    /* Convert the current time into local time */
    CurTime = time(NULL);
    LocTime = localtime(&CurTime);

    /* This will dereference LocTime even if it is NULL */
    BIO_snprintf(MsgBuff, sizeof(MsgBuff), "%02d-%s-%04d %02d:%02d:%02d [%08X] %s\n",
        LocTime->tm_mday, Month[LocTime->tm_mon],
        (LocTime->tm_year + 1900), LocTime->tm_hour, LocTime->tm_min,
        LocTime->tm_sec, pid, msg);

    /* Use variable args */
    va_start(args, msg);
    vfprintf(stderr, MsgBuff, args);
    va_end(args);

    /* Flush standard error output */
    fsync(fileno(stderr));
}

int main(void) {
    /* Trigger the bug: our interposed localtime() returns NULL */
    LogMessage((char *)"Test message: %d", 42);
    return 0;
}
