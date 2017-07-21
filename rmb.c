#include "Defs.h"
#include "LowLevelCommunication.h"
#include "HighLevelCommunication.h"
#include "ServerList.h"

int main(int argc, char **argv) {
  /* variables for identities' server communication */
  char *siip;
  int sipt = DEFAULT_SIPT;

  /* variables for msgserv communication */
  char *msgs_siip, *msgs_name;
  int msgs_sipt; 

  /* variables for udp client for id server */
  int id_fd, id_addrlen;
  struct hostent *id_hostptr;
  struct sockaddr_in id_serveraddr, id_clientaddr;

  /* variables for udp client */
  int fd, addrlen;
  struct hostent *hostptr;
  struct sockaddr_in serveraddr, clientaddr;

  /* auxiliar variables */
  int i, n = 0, quit = 0;
  char buffer[MAX_SIZE_STRING], command[MAX_SIZE_STRING], message[MAX_SIZE_STRING];
  ServerList *bl;

  /* set defaults */
  if(!(siip = (char *) malloc((strlen(DEFAULT_SIIP) + 1) * sizeof(char)))) {
    fprintf(stderr, "[FATAL] error while alocate memory\n");
    exit(-1);
  }
  strcpy(siip, DEFAULT_SIIP);

  /* take care of arguments */
  for(i = 1; i < argc; i++) {
    if(!strcmp(argv[i], "-i") && (i < argc - 1)) {
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
    else {
      fprintf(stderr, "[FATAL] invalid usage.\n");
      exit(-1);
    }

    i++;
  }

  /* ask for servers to identity server */
  if(init_udp(&id_fd, &id_hostptr, siip, sipt, &id_serveraddr, &id_clientaddr, &id_addrlen, client) == -1) {
    fprintf(stderr, "[FATAL] could not initialize udp.\n");
    exit(-1);
  }

  sprintf(buffer, "GET_SERVERS");

  /* ask for registered servers */
  if(request_udp(buffer, sizeof(buffer), buffer, id_fd, &id_serveraddr, &id_addrlen) == -1) {
    printf("[FATAL] identity server seems not to be alive\n");
    exit(-1);
  }

  initServerList(&bl);

  /* discard the first part of the answer */
  strtok(buffer, "\n");

  if(get_msgserv_identity(buffer, &msgs_name, &msgs_siip, &msgs_sipt, NULL, client) == -1) {
    fprintf(stderr, "[FATAL] there's no registered servers\n");
    exit(-1);
  }

  /* initialize udp */
  if(init_udp(&fd, &hostptr, msgs_siip, msgs_sipt, &serveraddr, &clientaddr, &addrlen, client) == -1) {
    fprintf(stderr, "[FATAL] could not initialize udp.\n");
    exit(-1);
  }

  /* main loop */
  while(!quit) {
    do {
      printf("%s > ", argv[0] + 2);
      fflush(stdout);

      if(!fgets(buffer, MAX_SIZE_STRING, stdin)) {
        fprintf(stderr, "[FATAL] error while reading from keyboard\n");
        exit(-1);
      }
    } while(sscanf(buffer, "%s", command) != 1);

    /* command was show_servers */
    if(!strcmp(command, "show_servers")) {
      if(get_and_show_servers(id_fd, &id_serveraddr, &id_addrlen) != 0)
        printf("[ERROR] could not show servers. Is the identity server alive?\n");
    }
    else if(!strcmp(command, "publish")) {
      for(i = 0; ; i++) {
        message[i] = buffer[i + strlen(command) + 1];
        if((buffer[i + strlen(command) + 1] == '\n') || (i == MAX_SIZE_MSG - 1)) {
          message[i] = '\0';
          break;
        }
      }
      
      if(publish(fd, message, &serveraddr, addrlen) != 0)
        printf("[ERROR] could not publish the message\n");
    }
    /* command was show_latest_messages */
    else if(!strcmp(command, "show_latest_messages")) {
      sscanf(buffer, "show_latest_messages %d", &n);

      do {
        if((i = show_last_messages(fd, &serveraddr, &addrlen, n)) != 0) {
          printf("[INFO] server went down, trying to reconnect\n");

          insertNodeServerList(bl, -1, msgs_sipt, -1, "incognito", msgs_siip);

          close(fd);

          sprintf(buffer, "GET_SERVERS");

          if(request_udp(buffer, sizeof(buffer), buffer, id_fd, &id_serveraddr, &id_addrlen) == -1) {
            printf("[FATAL] identity server seems not to be alive\n");
            exit(-1);
          }

          /* discard the first part of the answer */
          strtok(buffer, "\n");

          do {
            free(msgs_siip);
            free(msgs_name);

            if(get_msgserv_identity(buffer, &msgs_name, &msgs_siip, &msgs_sipt, NULL, client) == -1) {
              fprintf(stderr, "[FATAL] there's no registered servers\n");
              exit(-1);
            }
          }
          while(isInServerList(bl, msgs_sipt, msgs_siip));

          /* initialize udp */
          if(init_udp(&fd, &hostptr, msgs_siip, msgs_sipt, &serveraddr, &clientaddr, &addrlen, client) == -1) {
            fprintf(stderr, "[FATAL] could not initialize udp.\n");
            exit(-1);
          }
        }
      }
      while(i != 0);

      destroyServerList(&bl);
      initServerList(&bl);
    }
    /* command was exit */
    else if(!strcmp(command, "exit"))
      quit = 1;
    /* command was invalid */
    else
      printf("[ERROR] invalid command\n");
  }

  /* free memory */
  free(siip);
  free(msgs_siip);
  free(msgs_name);
  close(id_fd);
  close(fd);
  destroyServerList(&bl);

  exit(0);
}
