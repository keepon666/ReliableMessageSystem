CC		 = gcc
CFLAGS = -c -O3 -Wall

# client application
SOURCES_C    = rmb.c HighLevelCommunication.c LowLevelCommunication.c MessageList.c ServerList.c
EXECUTABLE_C = rmb
OBJECTS_C    = $(SOURCES_C:.c=.o)

# server application
SOURCES_S    = msgserv.c HighLevelCommunication.c LowLevelCommunication.c MessageList.c ServerList.c
EXECUTABLE_S = msgserv
OBJECTS_S    = $(SOURCES_S:.c=.o)

all: $(SOURCES_C) $(EXECUTABLE_C) $(SOURCES_S) $(EXECUTABLE_S)

$(EXECUTABLE_C): $(OBJECTS_C)
	$(CC) $(OBJECTS_C) -o $@

$(EXECUTABLE_S): $(OBJECTS_S)
	$(CC) $(OBJECTS_S) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@
	
clean:
	rm -rf *.o *.~ $(EXECUTABLE_C) $(EXECUTABLE_S) *.gch

backup:
	rm -rf backup.zip
	zip -r -9 proj31.zip *.c *.h makefile sid64 sid
