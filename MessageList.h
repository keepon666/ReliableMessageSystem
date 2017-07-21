#ifndef MESSAGE_LIST_HEADER_
#define MESSAGE_LIST_HEADER_

#include "Defs.h"

typedef struct MessageList_ {
  struct MessageNode_ *head, *tail;
  int maxSize, actualSize;
} MessageList;

typedef struct MessageNode_ {
  struct MessageNode_ *prev, *next;
  char *message;
  int logicCounter;
} MessageNode;

/* contructor and destructor */
void initMessageList(MessageList **ml, int maxSize);
void destroyMessageList(MessageList **ml);

/* getters and setters*/
MessageNode * getHeadMessageList(MessageList *ml);
MessageNode * getTailMessageList(MessageList *ml);
MessageNode * getNextNodeMessageList(MessageNode *node);
MessageNode * getPrevNodeMessageList(MessageNode *node);
char * getMessageMessageList(MessageNode *node);
int getLogicCounterMessageList(MessageNode *node);

void insertNodeMessageList(MessageList *ml, char *msg, int lc);
void deleteNodeMessageList(MessageList *ml, MessageNode *node);

/* auxiliar functions */
void deleteExcessMessageList(MessageList *ml);

#endif
