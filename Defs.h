#ifndef DEFS_HEADER_
#define DEFS_HEADER_

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <signal.h>
#include "string.h"

typedef enum{server, client} party;

#define STDIN_FD 0
#define MAX(A, B) (A > B) ? A : B

#define DEFAULT_SIIP "tejo.tecnico.ulisboa.pt"
#define DEFAULT_SIPT 59000

#define DEFAULT_MSGSERV_UDP_PORT 58000

#define MAX_SIZE_STRING 10000
#define MAX_SIZE_MSG 141
#define MAX_TIMEOUT 1

#endif
