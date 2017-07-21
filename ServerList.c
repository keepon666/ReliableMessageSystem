#include "ServerList.h"

/* contructor and destructor */
void initServerList(ServerList **sl) {
  if(!((*sl) = (ServerList *) malloc(sizeof(ServerList)))) {
    fprintf(stderr, "[FATAL] error while alocate memory\n");
    exit(-1);
  }

  (*sl)->actualSize = 0;
  (*sl)->head = (ServerNode *) NULL;
  (*sl)->tail = (ServerNode *) NULL;
}

void destroyServerList(ServerList **sl)
{
  ServerNode *prev, *next;

  if((*sl) == (ServerList *) NULL)
    return;

  prev = getHeadServerList(*sl);

  while(prev) {
    next = getNextNodeServerList(prev);
    deleteNodeServerList((*sl), prev);
    prev = next;
  }

  free(*sl);

  (*sl) = (ServerList *) NULL;
}

/* getters and setters*/
ServerNode * getHeadServerList(ServerList *sl) {
  return sl->head;
}

ServerNode * getTailServerList(ServerList *sl) {
  return sl->tail;
}

ServerNode * getNextNodeServerList(ServerNode *node) {
  if(node == (ServerNode *) NULL)
    return node;
  else
    return node->next;
}

ServerNode * getPrevNodeServerList(ServerNode *node) {
  if(node == (ServerNode *) NULL)
    return node;
  else
    return node->prev;
}

int getFileDescriptorServerList(ServerNode *node) {
  return node->fd;
}

int getUDPPortServerList(ServerNode *node) {
  return node->upt;
}

int getTCPPortServerList(ServerNode *node) {
  return node->tpt;
}

char * getNameServerList(ServerNode *node) {
  return node->name;
}

char * getIPServerList(ServerNode *node) {
  return node->ip;
}

int isInServerList(ServerList *sl, int msgs_sipt, char *msgs_siip) {
  ServerNode *node;

  for(node = getHeadServerList(sl); node != (ServerNode *) NULL; node = getNextNodeServerList(node))
    if((msgs_sipt == getUDPPortServerList(node)) && !strcmp(msgs_siip, getIPServerList(node)))
      return 1;

  return 0;
}

void insertNodeServerList(ServerList *sl, int fd, int upt, int tpt, char *name, char *ip) {
  ServerNode *newNode;

  if(!(newNode = (ServerNode *) malloc(sizeof(ServerNode)))) {
    fprintf(stderr, "[FATAL] error while alocate memory\n");
    exit(-1);
  }
  
  newNode->fd = fd;
  newNode->upt = upt;
  newNode->tpt = tpt;

  if(!(newNode->name = (char *) malloc((strlen(name) + 1) * sizeof(char)))) {
    fprintf(stderr, "[FATAL] error while alocate memory\n");
    exit(-1);
  }
  strcpy(newNode->name, name);

  if(!(newNode->ip = (char *) malloc((strlen(ip) + 1) * sizeof(char)))) {
    fprintf(stderr, "[FATAL] error while alocate memory\n");
    exit(-1);
  }
  strcpy(newNode->ip, ip);

  newNode->prev = getTailServerList(sl);
  newNode->next = (ServerNode *) NULL;

  /* the list is empty */
  if(sl->tail == (ServerNode *) NULL)
    sl->head = newNode;
  else
    sl->tail->next = newNode;

  sl->tail = newNode;

  sl->actualSize++;
}

void deleteNodeServerList(ServerList *sl, ServerNode *node) {
  ServerNode *prev, *next;

  prev = getPrevNodeServerList(node);
  next = getNextNodeServerList(node);

  /* it is not the head */
  if(prev != (ServerNode *) NULL)
    prev->next = next;
  else
    sl->head = next;

  /* it is not the tail */
  if(next != (ServerNode *) NULL)
    next->prev = prev;
  else
    sl->tail = prev;

  if(node->fd > 0)
    close(node->fd);

  free(node->name);
  free(node->ip);
  free(node);

  sl->actualSize--;
}
