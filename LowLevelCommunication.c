#include "LowLevelCommunication.h"

int init_udp(int *fd, struct hostent **hostptr, char *siip, int sipt, struct sockaddr_in *serveraddr, struct sockaddr_in *clientaddr, int *addrlen, party con) {
  (*fd) = socket(AF_INET, SOCK_DGRAM, 0);

  if(con == client) {
    if(((*hostptr) = gethostbyname(siip)) == (struct hostent *) NULL)
      return -1;
  }

  memset((void *) serveraddr, (int) '\0', sizeof(*serveraddr));

  serveraddr->sin_family = AF_INET;
  serveraddr->sin_addr.s_addr = (con == client) ? ((struct in_addr *) ((*hostptr)->h_addr_list[0]))->s_addr : htonl(INADDR_ANY);
  serveraddr->sin_port = htons((u_short) sipt);

  if(con == server)
    bind(*fd, (struct sockaddr*) serveraddr, sizeof(*serveraddr));

  (*addrlen) = (con == client) ? sizeof(*serveraddr) : sizeof(*clientaddr);

  return 0;
}

int init_tcp(int *fd, char *siip, int tpt, party con) {
  struct hostent *hostptr;
  struct sockaddr_in serveraddr;

  *fd = socket(AF_INET,SOCK_STREAM, 0);

  if(con == client)
    hostptr = gethostbyname(siip);

  memset((void*) &serveraddr,(int)'\0', sizeof(serveraddr));
  
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = (con == client) ? ((struct in_addr *) (hostptr->h_addr_list[0]))->s_addr : htonl(INADDR_ANY);
  serveraddr.sin_port = htons((u_short) tpt);

  if(con == client) {
    if(connect(*fd, (struct sockaddr*) &serveraddr, sizeof(serveraddr)) != 0)
      return -1;
  }
  else if(bind(*fd, (struct sockaddr*) &serveraddr, sizeof(serveraddr)) != 0 || listen(*fd, 5) != 0)
    return -1;

  return 0;
}