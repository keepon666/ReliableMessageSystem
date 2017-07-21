#include "MessageList.h"

/* contructor and destructor */
void initMessageList(MessageList **ml, int maxSize) {
  if(!((*ml) = (MessageList *) malloc(sizeof(MessageList)))) {
    fprintf(stderr, "[FATAL] error while alocate memory\n");
    exit(-1);
  }

  (*ml)->maxSize = maxSize;
  (*ml)->actualSize = 0;
  (*ml)->head = (MessageNode *) NULL;
  (*ml)->tail = (MessageNode *) NULL;
}

void destroyMessageList(MessageList **ml)
{
  MessageNode *prev, *next;

  if((*ml) == (MessageList *) NULL)
    return;

  prev = getHeadMessageList(*ml);

  while(prev) {
    next = getNextNodeMessageList(prev);
    deleteNodeMessageList((*ml), prev);
    prev = next;
  }

  free(*ml);

  (*ml) = (MessageList *) NULL;
}

/* getters and setters*/
MessageNode * getHeadMessageList(MessageList *ml) {
  return ml->head;
}

MessageNode * getTailMessageList(MessageList *ml) {
  return ml->tail;
}

MessageNode * getNextNodeMessageList(MessageNode *node) {
  if(node == (MessageNode *) NULL)
    return node;
  else
    return node->next;
}

MessageNode * getPrevNodeMessageList(MessageNode *node) {
  if(node == (MessageNode *) NULL)
    return node;
  else
    return node->prev;
}

char * getMessageMessageList(MessageNode *node) {
  return node->message;
}

int getLogicCounterMessageList(MessageNode *node) {
  return node->logicCounter;
}

void insertNodeMessageList(MessageList *ml, char *msg, int lc) {
  MessageNode *newNode, *prev, *next;

  if(strlen(msg) > 140)
    return;

  if(!(newNode = (MessageNode *) malloc(sizeof(MessageNode)))) {
    fprintf(stderr, "[FATAL] error while alocate memory\n");
    exit(-1);
  }
  if(!(newNode->message = (char *) malloc((strlen(msg) + 1) * sizeof(char)))) {
    fprintf(stderr, "[FATAL] error while alocate memory\n");
    exit(-1);
  }

  strcpy(newNode->message, msg);
  newNode->logicCounter = lc;

  for(next = getHeadMessageList(ml); (next != (MessageNode *) NULL) && (getLogicCounterMessageList(next) > lc); next = getNextNodeMessageList(next));

  newNode->next = next;

  /* insert at the end */
  if(next == (MessageNode *) NULL) {
    newNode->prev = getTailMessageList(ml);

    /* the list is not empty */
    if((prev = getTailMessageList(ml)) != (MessageNode *) NULL)
      prev->next = newNode;
    else
      ml->head = newNode;

    ml->tail = newNode;
  }
  else {
    newNode->prev = next->prev;
    next->prev = newNode;

    /* insert at the beginning */
    if((prev = newNode->prev) == (MessageNode *) NULL)
      ml->head = newNode;
    else
      prev->next = newNode;
  }

  ml->actualSize++;

  deleteExcessMessageList(ml);
}

void deleteNodeMessageList(MessageList *ml, MessageNode *node) {
  MessageNode *prev, *next;

  prev = getPrevNodeMessageList(node);
  next = getNextNodeMessageList(node);

  /* it is not the head */
  if(prev != (MessageNode *) NULL)
    prev->next = next;
  else
    ml->head = next;

  /* it is not the tail */
  if(next != (MessageNode *) NULL)
    next->prev = prev;
  else
    ml->tail = prev;

  free(node->message);
  free(node);

  ml->actualSize--;
}

/* auxiliar functions */
void deleteExcessMessageList(MessageList *ml) {
  while(ml->actualSize > ml->maxSize)
    deleteNodeMessageList(ml, getTailMessageList(ml));
}
