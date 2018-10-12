#include <sys/socket.h> // program interface to any network socket (generic)
#include <netinet/in.h> // specific to internet protocols (tcp, udp, idp)
#include <arpa/inet.h> // specific to internet protocols (tcp, udp, idp)
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>

/* Server half of client/server chat. */
int main(int argc, char** argv) {
	char port[1024];
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	int p;

	// check for error
	if (sockfd < 0) {
		printf("Problem creating socket\n");
		return 1;
	}

	// Check for valid port.
	printf("Please give a port number: \n");
	fgets(port, 1024, stdin);
	port[strcspn(port, "\n")] = 0;
	while(!check_port(port)) {
		printf("Please give a port number: \n");
		fgets(port, 1024, stdin);
		port[strcspn(port, "\n")] = 0;
	}

	p = atoi(port);

	// file descriptor set and set all to zero
	fd_set sockets;
	FD_ZERO(&sockets);

	struct sockaddr_in serveraddr, clientaddr;
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(p);
	serveraddr.sin_addr.s_addr=INADDR_ANY;

	int b = bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	
	// error check very important here
	if (b < 0) {
		printf("Bind error\n");
		return 3;
	}

	// 10 is the backlog for buffer of connection requests that we havent handled.
	listen(sockfd, 10);
	printf("Listening on server\n");
	//fflush(stdout);

	// put the socket in file descriptor.
	FD_SET(sockfd, &sockets);
	FD_SET(STDIN_FILENO, &sockets);

	// we want the server to keep accepting clients. an infinite loop is most common
	while(1) {
		fd_set tmp_set = sockets;
		select(FD_SETSIZE, &tmp_set, NULL, NULL, NULL);
		for (int i = 0; i < FD_SETSIZE; i++) {
			if (FD_ISSET(i, &tmp_set)) {
				if (i == sockfd) { // get new connection
					int len = sizeof(clientaddr);
					int clientsocket = accept(sockfd, (struct sockaddr*)&clientaddr, &len);
					FD_SET(clientsocket, &sockets);
					printf("Added client \n");
				}
				else if (i == STDIN_FILENO) { // am i sending a message?
					char line2[5000];
					fgets(line2, 5001, stdin);
					if (strcmp(line2, "exit\n") == 0) { // check for exit
						printf("Server shutting down.\n");
						for (int k = 0; k < FD_SETSIZE; k++) {
							send(k, line2, strlen(line2) + 1, 0);
						}
						close(sockfd);
						FD_CLR(sockfd, &sockets);
						exit(0);
					} else { // send that message
						for (int k = 0; k < FD_SETSIZE; k++) {
							send(k, line2, strlen(line2) + 1, 0);
						}
					}
				} else if (i) { // receive all messages
					char line[5000];
					recv(i, line, 5000, 0);
					printf("%s\n", line);
					if (strcmp(line, "exit\n") == 0) { // check for exit
						printf("Server shutting down.\n");
						for (int k = 0; k < FD_SETSIZE; k++) {
							send(k, line, strlen(line) + 1, 0);
						}
						close(sockfd);
						FD_CLR(sockfd, &sockets);
						exit(0);
					}
				}
			}
		}
	}
	return 0;
}

/* Check for valid digits in port number. */
int check_port(char* port) {
	int digits = 0;
	for (int i = 0; i < sizeof(port); i++) {
		if ((port[i] != '0') && !atoi(&port[i])) digits++;
	}
	if ((digits > 0 && digits < 6) && (atoi(port) > 0 && atoi(port) < 65536)) return 1;
	else return 0;
}