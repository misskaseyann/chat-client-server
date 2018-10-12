#include <sys/socket.h> // program interface to any network socket (generic)
#include <netinet/in.h> // specific to internet protocols (tcp, udp, idp)
#include <arpa/inet.h> // specific to internet protocols (tcp, udp, idp)
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define DELIM "."

int check_port(char*);

/* Client half of client/server chat. */
int main(int argc, char** argv) {
	char addr[1024], port[1024];
	unsigned long ip = 0;
	int p;

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd < 0) {
		printf("There was an error creating the socket\n");
		return 1;
	}

	// Check to make sure the address is valid, exit otherwise.
	printf("Please give an address: \n");
	fgets(addr, 1024, stdin);
	addr[strcspn(addr, "\n")] = 0;
	if (0 == inet_pton(AF_INET, addr, &ip)) {
        printf("Failed\n");
        exit(0);
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

	struct sockaddr_in serveraddr;
	serveraddr.sin_family=AF_INET;
	serveraddr.sin_port=htons(p); 
	serveraddr.sin_addr.s_addr=inet_addr(addr);

	int e = connect(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	
	if (e < 0) {
		printf("There was an error connecting\n");
		return 2;
	}

	printf("Client started \n");

	// Set file descriptors.
	fd_set clientfd;
	FD_ZERO(&clientfd);
	FD_SET(sockfd, &clientfd);
	FD_SET(STDIN_FILENO, &clientfd);

	while(1) {
		fd_set tmp_set = clientfd;
		select(FD_SETSIZE, &tmp_set, NULL, NULL, NULL);
		for (int i = 0; i < FD_SETSIZE; i++) {
			if (FD_ISSET(i, &tmp_set)) {
				if (i == sockfd) { // did i receive from server?
					char line[5000];
					recv(sockfd, line, 5000, 0);
					printf("%s\n", line);
					if (strcmp(line, "exit\n") == 0) { // check for exit
						printf("Server shutting down.\n");
						send(sockfd, line, strlen(line) + 1, 0);
						close(sockfd);
						FD_CLR(sockfd, &clientfd);
						exit(0);
					}
				} else if (i == STDIN_FILENO) { // am i sending a message?
					char line2[5000];
					fgets(line2, 5001, stdin);
					if (strcmp(line2, "exit\n") == 0) {
						printf("Server shutting down.\n");
						send(sockfd, line2, strlen(line2) + 1, 0);
						close(sockfd);
						FD_CLR(sockfd, &clientfd);
						exit(0);
					} else { // send that message
						send(sockfd, line2, strlen(line2) + 1, 0);
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
