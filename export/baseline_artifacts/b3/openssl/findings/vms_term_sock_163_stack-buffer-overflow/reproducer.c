// Standalone C reproducer for stack-buffer-overflow in vms_term_sock.c main
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

// ---- Stubs and definitions to mimic the original environment ----
#define TERM_SOCK_CREATE 1
#define TERM_SOCK_DELETE 2
#define TERM_SOCK_SUCCESS 1
#define TERM_SOCK_FAILURE 0

static int TerminalSocketPair[2] = { -1, -1 };

static void LogMessage(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
}

static int OPENSSL_strcasecmp(const char *a, const char *b) {
    return strcasecmp(a, b);
}

// Create a socket pair and pre-send exactly 80 bytes so recv() returns 80
static int CreateSocketPair(int domain, int type, int protocol, int sv[2]) {
    (void)domain; (void)protocol; // unused in this stub
    // Use AF_UNIX datagram sockets to preserve message boundaries
    if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sv) == -1) {
        return -1;
    }
    return 0;
}

int TerminalSocket(int FunctionCode, int *ReturnSocket) {
    int status;

    switch (FunctionCode) {
    case TERM_SOCK_CREATE: {
        status = CreateSocketPair(AF_UNIX, SOCK_DGRAM, 0, TerminalSocketPair);
        if (status == -1) {
            LogMessage("TerminalSocket: CreateSocketPair() failed: %s", strerror(errno));
            if (TerminalSocketPair[0] != -1) close(TerminalSocketPair[0]);
            if (TerminalSocketPair[1] != -1) close(TerminalSocketPair[1]);
            TerminalSocketPair[0] = TerminalSocketPair[1] = -1;
            return TERM_SOCK_FAILURE;
        }

        // Pre-send exactly 80 bytes so that recv() returns 80
        char payload[80];
        memset(payload, 'A', sizeof(payload));
        ssize_t sent = send(TerminalSocketPair[1], payload, sizeof(payload), 0);
        if (sent != (ssize_t)sizeof(payload)) {
            LogMessage("Failed to send 80 bytes on socketpair: sent=%zd errno=%s", sent, strerror(errno));
        }

        // Return the receiving end
        *ReturnSocket = TerminalSocketPair[0];
        return TERM_SOCK_SUCCESS;
    }
    case TERM_SOCK_DELETE:
        if (TerminalSocketPair[0] != -1) close(TerminalSocketPair[0]);
        if (TerminalSocketPair[1] != -1) close(TerminalSocketPair[1]);
        TerminalSocketPair[0] = TerminalSocketPair[1] = -1;
        if (ReturnSocket) *ReturnSocket = -1;
        return TERM_SOCK_SUCCESS;
    default:
        return TERM_SOCK_FAILURE;
    }
}

// ---- Vulnerable logic reproduced from apps/lib/vms_term_sock.c (TERM_SOCK_TEST main) ----
int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    char TermBuff[80];
    int TermSock, status, len;

    // Initialize to non-"Q" so the loop runs once
    TermBuff[0] = 'X';
    TermBuff[1] = '\0';

    LogMessage("Enter 'q' or 'Q' to quit ...");
    while (OPENSSL_strcasecmp(TermBuff, "Q")) {
        // Create the terminal socket (our stub pre-sends 80 bytes)
        status = TerminalSocket(TERM_SOCK_CREATE, &TermSock);
        if (status != TERM_SOCK_SUCCESS)
            exit(1);

        LogMessage("Waiting on terminal I/O ...");
        // This recv will return exactly 80, filling TermBuff completely
        len = recv(TermSock, TermBuff, sizeof(TermBuff), 0);

        // VULNERABILITY: No validation of len. If len == 80, this writes 1 past end
        TermBuff[len] = '\0';

        LogMessage("Received terminal I/O [%s]", TermBuff);

        status = TerminalSocket(TERM_SOCK_DELETE, &TermSock);
        if (status != TERM_SOCK_SUCCESS)
            exit(1);

        // Not reached if ASan aborts on overflow; otherwise exit cleanly
        break;
    }

    return 0;
}
