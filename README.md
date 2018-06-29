# ReliableMessageSystem

This project had as the main goal to develop a basic broadcast communication system using nothing else than C language and TCP-IP and UDP protocols.

There are three types of entities involved in this system:
1. Clients;
2. Message servers;
3. Entity servers.

In order for the clients to be capable of communicating, there should be only an entity server and at least one message server. When the message server starts, it tries to register in the entity server. If it succeeds, all the message history will be asked to one of its peers (another message server). Then it becomes ready to accept connections from clients. When one client connects, it asks for the address of a random message server to the entity server, and it establishes a connection with it. After all the three entities are online, clients are capable of sending and receiving messages from all to all. The system also includes some redundancy (e.g., when a message server goes offline, its clients will try to reconnect to another server).

More details about this project can be found [here](https://github.com/joaomiguelvieira/ReliableMessageSystem/blob/master/ReliableMessageBoard.pdf).
