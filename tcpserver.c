#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>

#define bufsize 1024

void eatZombies(int n){
	// This function removes the zombie process state left
	// after the child exits. Learn about this in NWEN 301!

	wait3(NULL,WNOHANG,NULL); // Nom Nom
}

int main(int argc, char *argv[]){
	int sock, length, msgsock, status;
	struct sockaddr_in server;

	// for forking, and cleaning up zombie child state afterwards
	// You must not change this code.

	pid_t id;
	signal(SIGCHLD, &eatZombies);

	// OK, NWEN 243 code starts here.

	if(argc != 2){
		printf("Usage:\ntcpserver port\n");
		exit(3);
	}

	// Create a socket (see Lab 2) - it is exactly the same for a server!
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) { perror("opening socket");  return(-1); }

	// Next, create the socket addressing structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(atoi(argv[1])); // this time 1st arg is port#

	// Next you need to BIND your socket.
	if( bind(sock, (struct sockaddr*) &server, sizeof(server) ) < 0){
		perror("opening socket");
		exit(1);
	}

	// Right, now we are in the main server loop.

	// Start by listening

	listen(sock, 5);
	length = sizeof(server);
	char buf[bufsize];

	while(1){

		// Accept the connection request
		msgsock = accept(sock, (struct sockaddr*) &server, &length );
		if( msgsock < 0 ){
			perror("error accepting connection");
			exit(1);
		}
		length = sizeof(msgsock);

		// the next call makes a new child process that will actually handle the client.
		id = fork();

		// when id == 0, this is the child and needs to do the work for the server.
		// when if > 0, this is the parent, and it should just loop around,
		// when id < 0, we had an error.
		if(id < 0){ perror("id < 0"); exit(1); }

		// Child process
		else if(id == 0){
			// Read message from msgsock and put it into buf
			if(read(msgsock, buf, bufsize) < 0){
				perror("error receiving packet");
				exit(1);
			}

			// Print out message received from client
			fprintf(stdout, "client says: %s\n", buf);

			// Convert each character in buf to uppercase with toupper
			int i = 0;
			while(i < bufsize){
				buf[i] = toupper(buf[i]);
				i++;
			}

			// Write message back to msgsock from buf
			if(write(msgsock, buf, bufsize) < 0){
				perror("error sending message");
				exit(1);
			}
			// Close connection
			close(msgsock);
		}

		else if(id > 0){ //this is the parent, do nothing
		}
	}
}

