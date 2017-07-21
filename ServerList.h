#ifndef SERVER_LIST_HEADER_
#define SERVER_LIST_HEADER_

#include "Defs.h"

typedef struct ServerList_ {
  struct ServerNode_ *head, *tail;
  int actualSize;
} ServerList;

typedef struct ServerNode_ {
  struct ServerNode_ *prev, *next;
  int fd, upt, tpt;
  char *name, *ip;
} ServerNode;

/* contructor and destructor */
void initServerList(ServerList **sl);
void destroyServerList(ServerList **sl);

/* getters and setters*/
ServerNode * getHeadServerList(ServerList *sl);
ServerNode * getTailServerList(ServerList *sl);
ServerNode * getNextNodeServerList(ServerNode *node);
ServerNode * getPrevNodeServerList(ServerNode *node);
int getFileDescriptorServerList(ServerNode *node);
int getUDPPortServerList(ServerNode *node);
int getTCPPortServerList(ServerNode *node);
char * getNameServerList(ServerNode *node);
char * getIPServerList(ServerNode *node);
int isInServerList(ServerList *sl, int msgs_sipt, char *msgs_siip);

void insertNodeServerList(ServerList *sl, int fd, int upt, int tpt, char *name, char *ip);
void deleteNodeServerList(ServerList *sl, ServerNode *node);

#endif
