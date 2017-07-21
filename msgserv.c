#include "Defs.h"
#include "LowLevelCommunication.h"
#include "HighLevelCommunication.h"

#include "MessageList.h"
#include "ServerList.h"

#define DEFAULT_M 200
#define DEFAULT_R 10

typedef void (*sighandler_t)(int);
sighandler_t signal(int signum, sighandler_t handler);

int main(int argc, char ** argv) {
  /* main variables */
  char *name = NULL, *ip = NULL, *siip;
  int upt = -1, tpt = -1, sipt = DEFAULT_SIPT, m = DEFAULT_M, r = DEFAULT_R;

  char buffer[MAX_SIZE_STRING], command[MAX_SIZE_STRING], buffer2[MAX_SIZE_STRING];
  int n;

  MessageList *ml;
  ServerList *sl;
  char message[MAX_SIZE_STRING], command_msg[MAX_SIZE_STRING];
  int joined = 0, lc = 0;

  /* variables for udp client */
  int id_fd, id_addrlen;
  struct hostent *id_hostptr;
  struct sockaddr_in id_serveraddr, id_clientaddr;

  /* variables for udp server */
  int fd, addrlen;
  struct sockaddr_in serveraddr, clientaddr;

  /* variables for tcp client and server*/
  int my_tcp_fd, tcp_clientlen, tcp_newFd;
  struct sockaddr_in tcp_clientaddr;

  /* multiplexing variables */
  fd_set rfds;
  int maxfd, counter;
  struct timeval timeout;
  time_t prevTime, actualTime, cycleDelay;

  /* auxiliar variables */
  int i, quit = 0;
  MessageNode *node;
  ServerNode *snode;
  char *aux;
  int nread, nwritten, nleft, nTRead;
  char buffer_smsg[MAX_SIZE_STRING];
  void (*old_handler)(int);

  /* deal with SIGPIPE */
  if((old_handler=signal(SIGPIPE,SIG_IGN)) == SIG_ERR)
    exit(-1);

  /* set defaults */
  if(!(siip = (char *) malloc((strlen(DEFAULT_SIIP) + 1) * sizeof(char)))) {
    fprintf(stderr, "[FATAL] error while alocate memory\n");
    exit(-1);
  }
  strcpy(siip, DEFAULT_SIIP);

  /* arguments parsing */
  for(i = 1; i < argc; i++) {
    if(!strcmp(argv[i], "-n") && (i < argc - 1)) {
      if(!(name = (char *) malloc((strlen(argv[i + 1]) + 1) * sizeof(char)))) {
        fprintf(stderr, "[FATAL] error while alocate memory\n");
        exit(-1);
      }
      strcpy(name, argv[i + 1]);
    }
    else if(!strcmp(argv[i], "-j") && (i < argc - 1)) {
      if(!(ip = (char *) malloc((strlen(argv[i + 1]) + 1) * sizeof(char)))) {
        fprintf(stderr, "[FATAL] error while alocate memory\n");
        exit(-1);
      }
      strcpy(ip, argv[i + 1]);
    }
    else if(!strcmp(argv[i], "-u") && (i < argc - 1)) {
      upt = atoi(argv[i + 1]);
    }
    else if(!strcmp(argv[i], "-t") && (i < argc - 1)) {
      tpt = atoi(argv[i + 1]);
    }
    else if(!strcmp(argv[i], "-i") && (i < argc - 1)) {
      free(siip);
      if(!(siip = (char *) malloc((strlen(argv[i + 1]) + 1) * sizeof(char)))) {
        fprintf(stderr, "[FATAL] error while alocate memory\n");
        exit(-1);
      }
      strcpy(siip, argv[i + 1]);
    }
    else if(!strcmp(argv[i], "-p") && (i < argc - 1)) {
      sipt = atoi(argv[i + 1]);
    }
    else if(!strcmp(argv[i], "-m") && (i < argc - 1)) {
      m = atoi(argv[i + 1]);
    }
    else if(!strcmp(argv[i], "-r") && (i < argc - 1)) {
      r = atoi(argv[i + 1]);
    }
    else {
      fprintf(stderr, "[FATAL] invalid usage.\n");
      exit(-1);
    }

    i++;
  }

  if((name == NULL) || (ip == NULL) || (upt == -1) || (tpt == -1)) {
    fprintf(stderr, "[FATAL] invalid usage.\n");
    exit(-1);
  }

  /* initialize message and server lists */
  initMessageList(&ml, m);
  initServerList(&sl);

  /* initialize udp server */
  if(init_udp(&fd, (struct hostent **) NULL, (char *) NULL, upt, &serveraddr, &clientaddr, &addrlen, server) == -1) {
    fprintf(stderr, "[FATAL] could not initialize udp server.\n");
    exit(-1);
  }

  /* establish tcp connections to each of registered server */

  /* just to initialize the shell */
  printf("%s > ", argv[0] + 2);
  fflush(stdout);

  while(!quit) {
    /* select to multiplex */
    FD_ZERO(&rfds);

    /* keyboard */
    FD_SET(STDIN_FD, &rfds);
    
    /* udp server */
    FD_SET(fd, &rfds);
    maxfd = MAX(STDIN_FD, fd);

    if(joined) {
      /* tcp server */
      FD_SET(my_tcp_fd, &rfds);
      maxfd = MAX(maxfd, my_tcp_fd);
    
      /* tcp clients */
      for(snode = getHeadServerList(sl); snode != (ServerNode *) NULL; snode = getNextNodeServerList(snode)) {
        FD_SET(getFileDescriptorServerList(snode), &rfds);
        maxfd = MAX(maxfd, getFileDescriptorServerList(snode));
      }
    }

    /* counter means the number of descriptors that are active */
    /* the timer is only set when the server is joined */
    counter = joined ? select(maxfd + 1, &rfds, (fd_set *) NULL, (fd_set *) NULL, &timeout) : select(maxfd + 1, &rfds, (fd_set *) NULL, (fd_set *) NULL, (struct timeval *) NULL);

    /* if select unlocked with number minor than 0 then it is an error */
    if(counter < 0) {
      fprintf(stderr, "[FATAL] select unlocked with negative file descriptor.\n");
      exit(-1);
    }
    /* if counter greater than 0 then there is a file descriptor needing attention*/
    else if(counter > 0) {
      /* input from keyboard */
      if(FD_ISSET(STDIN_FD, &rfds)) {
        for(i = 0; i < MAX_SIZE_STRING; i++)
          command[i] = '\0';

        if(!fgets(buffer, MAX_SIZE_STRING, stdin));

        sscanf(buffer, "%s", command);

        /* command was join */
        if(!joined && !strcmp(command, "join")) {
          if(join(&id_fd, &id_hostptr, siip, sipt, &id_serveraddr, &id_clientaddr, &id_addrlen, name, ip, upt, tpt, &joined, &prevTime, &timeout, r, sl, &my_tcp_fd) != 0) {
            printf("[ERROR] could not join the server\n");
          }
          else {
            snode = getHeadServerList(sl);

            if(snode != (ServerNode *) NULL) {
              sprintf(buffer_smsg, "SGET_MESSAGES");

              nleft = strlen(buffer_smsg) + 1;
              buffer_smsg[nleft - 1] = '\n';
              nwritten = 0;

              do {
                if((nwritten = write(getFileDescriptorServerList(snode), buffer_smsg + nwritten, nleft)) == -1) {
                  deleteNodeServerList(sl, snode);
                  break;
                }

                nleft -= nwritten;
              }
              while(nleft > 0);
            }
          }
        }
        /* command was show_servers */
        else if(!strcmp(command, "show_servers")) {
          if(show_servers(sl) != 0)
            printf("[ERROR] could not show servers\n");
        }
        /* command was show_messages */
        else if(!strcmp(command, "show_messages")) {
          if(show_messages(ml) != 0)
            printf("[ERROR] could not show messages\n");
        }
        /* command was exit */
        else if(!strcmp(command, "exit")) {
          quit = 1;
        }
        /* command is not valid */
        else if(command[0] != '\0')
          printf("[ERROR] invalid command\n");

        printf("%s > ", argv[0] + 2);
        fflush(stdout);

        /* clean previous content of command not to be repeated */
        command[0] = '\0';
      }
      /* udp server client sent a message/request */
      else if(FD_ISSET(fd, &rfds)) {
        for(i = 0; i < MAX_SIZE_STRING; i++)
          buffer[i] = '\0';

        if(recvfrom(fd, buffer, sizeof(buffer), 0, (struct sockaddr*) &clientaddr, (socklen_t *) &addrlen) < 1) {
          printf("[FATAL] recvfrom returned error\n");
        }
        sscanf(buffer, "%s", command_msg);

        aux = strtok(buffer + strlen(command_msg) + 1, "\n");

        /* it was a publish */
        if(!strcmp(command_msg, "PUBLISH")) {
          insertNodeMessageList(ml, aux, lc);

          sprintf(buffer_smsg, "SMESSAGES\n%d;%s\n\n", lc, aux);

          i = strlen(buffer_smsg);
          buffer_smsg[i - 1] = '\n';

          for(snode = getHeadServerList(sl); snode != (ServerNode*) NULL; snode = getNextNodeServerList(snode)) {
            nleft = i;
            nwritten = 0;

            do {
              /* server is dead */
              if((nwritten = write(getFileDescriptorServerList(snode), buffer_smsg + nwritten, nleft)) == -1) {
                deleteNodeServerList(sl, snode);
                break;
              }

              nleft -= nwritten;
            }
            while(nleft > 0);
          }

          lc++;
        }
        /* it was a request of last messages */
        else if(!strcmp(command_msg, "GET_MESSAGES")) {
          sscanf(buffer + strlen(command_msg) + 1, "%d", &n);

          strcpy(message, "MESSAGES\n");
          node = getHeadMessageList(ml);
          for(i = 1; i < n; i++) {
            if(node == (MessageNode *) NULL || getNextNodeMessageList(node) == (MessageNode *) NULL)
              break;
            else
              node = getNextNodeMessageList(node);
          }

          while(node && n) {
            sprintf(buffer, "%s\n", getMessageMessageList(node));
            strcat(message, buffer);
            node = getPrevNodeMessageList(node);
          }

          sendto(fd, message, strlen(message) + 1, 0, (struct sockaddr *) &clientaddr, addrlen);
        }
      }
      /* an incoming connection */
      else if(FD_ISSET(my_tcp_fd, &rfds)) {
        tcp_clientlen = sizeof(tcp_clientaddr);
        tcp_newFd = accept(my_tcp_fd, (struct sockaddr *) &tcp_clientaddr, (socklen_t *) &tcp_clientlen);

        sprintf(buffer, "%d.%d.%d.%d", (int) (tcp_clientaddr.sin_addr.s_addr&0xFF), (int) ((tcp_clientaddr.sin_addr.s_addr&0xFF00)>>8), (int) ((tcp_clientaddr.sin_addr.s_addr&0xFF0000)>>16), (int) ((tcp_clientaddr.sin_addr.s_addr&0xFF000000)>>24));

        insertNodeServerList(sl, tcp_newFd, -1, (int) tcp_clientaddr.sin_port, "incognito", buffer);
      }
      else
        /* a msg server with which a tcp connection is established requires attention */
        for(snode = getHeadServerList(sl); snode != (ServerNode *) NULL; snode = getNextNodeServerList(snode)) {
          if(FD_ISSET(getFileDescriptorServerList(snode), &rfds)) {
            nread = 0;
            nleft = MAX_SIZE_STRING;

            for(i = 0; i < MAX_SIZE_STRING; i++)
              buffer[i] = '\0';

            i = 0;
            nTRead = 0;

            do {
              /* if it returns 0 then it was a fin, less than 1 it was an error */
              if((nread = read(getFileDescriptorServerList(snode), buffer + nTRead, nleft)) <= 0) {
                deleteNodeServerList(sl, snode);
                break;
              }

              nleft -= nread;
              nTRead += nread;

              strcpy(buffer2, buffer);

              aux = strtok(buffer2, "\n");
              if(nTRead >= 2) {
                if(aux != NULL && !strcmp(aux, "SMESSAGES") && buffer[nTRead - 1] == '\n' && buffer[nTRead - 2] == '\n')
                  i = 1;
                else if(aux != NULL && !strcmp(aux, "SGET_MESSAGES") && buffer[strlen("SGET_MESSAGES")] == '\n')
                  i = 1;
                else if(aux == NULL || (strcmp(aux, "SMESSAGES") && strcmp(aux, "SGET_MESSAGES")))
                  i = 1;
              }
              else {
                i = 1;
              }
            }
            while(i != 1);

            for(i = 1; i < MAX_SIZE_STRING; i++)
              if(buffer[i - 1] == '\n' && buffer[i] == '\n') {
                buffer[i] = '\0';
                break;
              }

            aux = strtok(buffer,"\n");

            /* parse messages */
            if(aux != (char *) NULL && !strcmp(aux, "SMESSAGES")) {
              while((aux = strtok(NULL, ";"))) {
                i = atoi(aux);
                
                if(!(aux = strtok(NULL, "\n")))
                  break;

                insertNodeMessageList(ml, aux, i);
                lc = MAX(lc, i) + 1;
              }
            }
            /* answer to sget_messages request */
            else if(aux != (char *) NULL && !strcmp(aux, "SGET_MESSAGES")) {
              sprintf(buffer_smsg, "SMESSAGES\n");

              for(node = getTailMessageList(ml); node != (MessageNode *) NULL; node = getPrevNodeMessageList(node)) {
                sprintf(buffer, "%d;%s\n", getLogicCounterMessageList(node), getMessageMessageList(node));
                strcat(buffer_smsg, buffer);
              }

              nleft = strlen(buffer_smsg) + 1;
              buffer_smsg[nleft - 1] = '\n';
              nwritten = 0;

              do {
                /* server is dead */
                if((nwritten = write(getFileDescriptorServerList(snode), buffer_smsg + nwritten, nleft)) == -1) {
                  deleteNodeServerList(sl, snode);
                  break;
                }

                nleft -= nwritten;
              }
              while(nleft > 0);
            }

            break;
          }
        }
    }

    /* reset timer */
    if(joined) {
      actualTime = time((time_t *) NULL);

      if((cycleDelay = (int) difftime(actualTime, prevTime)) < r)
        timeout.tv_sec = r - cycleDelay;
      else {
        timeout.tv_sec = r;
        prevTime = time((time_t *) NULL);
        
        if(register_id_server(id_fd, &id_serveraddr, id_addrlen, name, ip, upt, tpt) == -1) {
          fprintf(stderr, "[FATAL] could not register into identity server\n");
          exit(-1);
        }
      }
    }
  }

  /* free memory */
  free(siip);
  free(name);
  free(ip);
  destroyMessageList(&ml);
  destroyServerList(&sl);
  
  if(joined) {
    close(id_fd);
    close(my_tcp_fd);
  }
  
  close(fd);

  exit(0);
}
