#ifndef HIGH_LEVEL_COMMUNICATION_HEADER_
#define HIGH_LEVEL_COMMUNICATION_HEADER_

#include "Defs.h"
#include "MessageList.h"
#include "ServerList.h"
#include "LowLevelCommunication.h"

int register_id_server(int id_fd, struct sockaddr_in *id_serveraddr, int id_addrlen, char * name, char * ip, int upt, int tpt);
int join(int *id_fd, struct hostent **id_hostptr, char * siip, int sipt, struct sockaddr_in *id_serveraddr, struct sockaddr_in *id_clientaddr, int *id_addrlen, char * name, char * ip, int upt, int tpt, int *joined, time_t *prevTime, struct timeval *timeout, int r, ServerList *sl, int *my_utp_fd);
int show_servers(ServerList *sl);
int show_messages(MessageList *ml);

int get_and_show_servers(int fd, struct sockaddr_in *serveraddr, int *addrlen);
int publish(int fd, char *message, struct sockaddr_in *serveraddr, int addrlen);
int show_last_messages(int fd, struct sockaddr_in *serveraddr, int *addrlen, int n);
int get_msgserv_identity(char *buffer, char **name, char **ip, int *upt, int *tpt, party con);
int request_udp(char *buffer, int size, char *request, int fd, struct sockaddr_in *serveraddr, int *addrlen);

#endif