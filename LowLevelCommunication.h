#ifndef LOW_LEVEL_COMMUNICATION_HEADER_
#define LOW_LEVEL_COMMUNICATION_HEADER_

#include "Defs.h"

int init_udp(int *fd, struct hostent **hostptr, char *siip, int sipt, struct sockaddr_in *serveraddr, struct sockaddr_in *clientaddr, int *addrlen, party con);
int init_tcp(int *fd, char *siip, int tpt, party con);

#endif