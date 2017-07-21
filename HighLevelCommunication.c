#include "HighLevelCommunication.h"

/* server functions */
int register_id_server(int id_fd, struct sockaddr_in *id_serveraddr, int id_addrlen, char * name, char * ip, int upt, int tpt) {
  char msgtosend[MAX_SIZE_STRING];
  
  sprintf(msgtosend, "REG %s;%s;%d;%d", name, ip, upt, tpt);
  sendto(id_fd, msgtosend, strlen(msgtosend) + 1, 0, (struct sockaddr*) id_serveraddr, id_addrlen);

  return 0;
}

int join(int *id_fd, struct hostent **id_hostptr, char * siip, int sipt, struct sockaddr_in *id_serveraddr, struct sockaddr_in *id_clientaddr, int *id_addrlen, char * name, char * ip, int upt, int tpt, int *joined, time_t *prevTime, struct timeval *timeout, int r, ServerList *sl, int *my_tcp_fd) {
  char buffer[MAX_SIZE_STRING], *msgs_siip, *msgs_name;
  int msgs_upt, msgs_tpt, newFd;

  /* initialize udp client */
  if(init_udp(id_fd, id_hostptr, siip, sipt, id_serveraddr, id_clientaddr, id_addrlen, client) == -1) {
    fprintf(stderr, "[FATAL] could not initialize udp client.\n");
    exit(-1);
  }

  /* register the server in identity server */
  if(register_id_server(*id_fd, id_serveraddr, *id_addrlen, name, ip, upt, tpt) == -1) {
    return -1;
  }

  /* get servers */
  sprintf(buffer, "GET_SERVERS");

  if(request_udp(buffer, sizeof(buffer), buffer, *id_fd, id_serveraddr, id_addrlen) == -1) {
    printf("[FATAL] identity server seems not to be alive\n");
    exit(-1);
  }

  /* eliminate protocol's header */
  strtok(buffer, "\n");

  /* initialize a tcp connection for every msgserv registered */
  while(get_msgserv_identity(buffer, &msgs_name, &msgs_siip, &msgs_upt, &msgs_tpt, server) != -1) {
    if(init_tcp(&newFd, msgs_siip, msgs_tpt, client) != -1)
      insertNodeServerList(sl, newFd, msgs_upt, msgs_tpt, msgs_name, msgs_siip);

    free(msgs_siip);
    free(msgs_name);
  }

  /* iniliatize tcp server */
  if(init_tcp(my_tcp_fd, (char *) NULL, tpt, server) != 0) {
    fprintf(stderr, "[FATAL] could not initialize TCP listening\n");
    exit(-1);
  }

  /* set timer and joined */
  (*joined) = 1;
  (*prevTime) = time((time_t *) NULL);
  timeout->tv_sec = r;
  timeout->tv_usec = 0;

  return 0;
}

int show_servers(ServerList *sl) {
  ServerNode *node;

  for(node = getHeadServerList(sl); node != (ServerNode *) NULL; node = getNextNodeServerList(node))
    printf("%s %s %d %d\n", getNameServerList(node), getIPServerList(node), getUDPPortServerList(node), getTCPPortServerList(node));

  return 0;
}

int show_messages(MessageList *ml) {
  MessageNode *node;

  for(node = getTailMessageList(ml); node != (MessageNode *) NULL; node = getPrevNodeMessageList(node))
    printf("%d: %s\n", getLogicCounterMessageList(node), getMessageMessageList(node));

  return 0;
}

/* client functions */


int get_and_show_servers(int fd, struct sockaddr_in *serveraddr, int *addrlen) {
  char msgtosend[MAX_SIZE_STRING], buffer[MAX_SIZE_STRING], *aux;
  int i;

  /* clean the buffer */
  for(i = 0; i < MAX_SIZE_STRING; i++)
    buffer[i] = '\0';
  
  sprintf(msgtosend, "GET_SERVERS");

  /* get servers */
  if(request_udp(buffer, sizeof(buffer), msgtosend, fd, serveraddr, addrlen) == -1) {
    printf("[FATAL] identity server went down\n");
    exit(-1);
  }

  /* eliminate protocol's header */
  strtok(buffer, "\n");

  while((aux = strtok(NULL, "\n")) && strlen(aux))
    printf("%s\n", aux);

  return 0;
}

int publish(int fd, char *message, struct sockaddr_in *serveraddr, int addrlen) {
  char msgtosend[MAX_SIZE_STRING];
  
  sprintf(msgtosend, "PUBLISH %s", message);
  sendto(fd, msgtosend, strlen(msgtosend) + 1, 0, (struct sockaddr*) serveraddr, addrlen);

  return 0;
}

int show_last_messages(int fd, struct sockaddr_in *serveraddr, int *addrlen, int n) {
  char request[MAX_SIZE_STRING], buffer[MAX_SIZE_STRING], *aux;
  int i;
  
  sprintf(request, "GET_MESSAGES %d", n);

  /* clean the buffer */
  for(i = 0; i < MAX_SIZE_STRING; i++)
    buffer[i] = '\0';

  /* send an UDP message and wait MAX_TIMEOUT for an answer */
  if(request_udp(buffer, sizeof(buffer), request, fd, serveraddr, addrlen) == -1) {
    return -1;
  }
  else {
    /* eliminate protocol's header */
    strtok(buffer, "\n");

    while((aux = strtok(NULL, "\n")) && strlen(aux))
      printf("%s\n", aux);
  }

  return 0;
}

int get_msgserv_identity(char *buffer, char **name, char **ip, int *upt, int *tpt, party con) {
  int i;
  char *seg[4];

  /* get the different fields that composes server's identity */
  seg[3] = strtok(NULL, ";");
  seg[0] = strtok(NULL, ";");
  seg[1] = strtok(NULL, ";");
  seg[2] = strtok(NULL, "\n");

  for(i = 0; i < 3; i++) {
    if(seg[i] == NULL)
      return -1;
  }

  if(!((*ip) = (char *) malloc((strlen(seg[0]) + 1) * sizeof(char)))) {
    fprintf(stderr, "[FATAL] error while alocate memory\n");
    exit(-1);
  }
  strcpy(*ip, seg[0]);

  if(!((*name) = (char *) malloc((strlen(seg[3]) + 1) * sizeof(char)))) {
    fprintf(stderr, "[FATAL] error while alocate memory\n");
    exit(-1);
  }
  strcpy(*name, seg[3]);

  (*upt) = atoi(seg[1]);
  if(con == server)
    (*tpt) = atoi(seg[2]);

  return 0;
}

int request_udp(char *buffer, int size, char *request, int fd, struct sockaddr_in *serveraddr, int *addrlen) {
  fd_set rfds;
  struct timeval timeout;
  int counter;

  /* send query */
  sendto(fd, request, strlen(request), 0, (struct sockaddr*) serveraddr, *addrlen);

  /* prepare timeout */
  FD_ZERO(&rfds);
  FD_SET(fd, &rfds);
  timeout.tv_sec = MAX_TIMEOUT;
  timeout.tv_usec = MAX_TIMEOUT;

  counter = select(fd + 1, &rfds, (fd_set *) NULL, (fd_set *) NULL, &timeout);

  /* timeout occured */
  if(counter == 0)
    return -1;
  else
    /* the other party answered */
    if(recvfrom(fd, buffer, size, 0, (struct sockaddr*) serveraddr, (socklen_t *) addrlen) < 1) {
      printf("[FATAL] recvfrom returned error\n");
    }

  return 0;
}
