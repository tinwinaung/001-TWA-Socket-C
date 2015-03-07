#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h> /* memset() */
#include <time.h>
#include <sys/time.h> /* select() */ 

/* Definations */
#define EXIT_SUCCESS	0
#define EXIT_FAILURE	1

#define MAX_MSG 100
#define SOCKET_ERROR -1

//void Broadcast();
int isReadable(int sd, int * error, int timeOut);

void getInput(char* inp, int size_inp);
void errorExit(char *errorMessage);
void clearScreen();
void AppHeadLine();
void iSleep(float milis);

int main(int argc, char *argv[]) {
	int sockfd;
	struct sockaddr_in their_addr; // connector's address information
	struct hostent *he;
	int numbytes;
	char okMsg[] = "OK";
	char initMsg[] = "init"; // Broadcast message in this protocol
	char readyMsg[] = "ready"; // Server reply ready to serve
	char reqMsg[] = "req"; // Client request
	char charPort[50]; //store user input for port
	char charBroadCastMask[30]; //store user input for port
	char serverAddr[30];

//	if (argc != 3) {
//		errorExit("usage: client subnet_mask port\n");
//	}
	//Ask port number for server
	clearScreen();
	AppHeadLine();
	printf("Please enter server port to connect\nPort[9999]:");
	getInput(charPort, sizeof charPort);
	if (strlen(charPort) == 0) {
		printf("Client need port.\n- Client will use 9999 as default.\n");
		strcpy(charPort, "9999"); // use 9999 as default port. Assign it to port since user have no input
		//TODO: We will need to check more. E.g: check integer.
	}
	//
	//Add server addr
//	printf("Please enter Server Addr\nAddr[127.0.0.1]:");
//	getInput(serverAddr, sizeof serverAddr);
//	if (strlen(serverAddr) == 0) {
//		printf(
//				"Client need server addr.\n- Client will use 127.0.0.1 as default.\n");
//		strcpy(serverAddr, "127.0.0.1"); // use 127.0.0.1 as default Addr. Assign it to Addr since user have no input
//		//TODO: We will need to check more. E.g: Check IP format.
//	}
	//Ask broadcast Address
	printf("Please enter Broadcast Addr\nAddr[255.255.255.255]:");
	//http://en.wikipedia.org/wiki/Broadcast_address
	getInput(charBroadCastMask, sizeof charBroadCastMask);
	if (strlen(charBroadCastMask) == 0) {
		printf(
				"Client need Broadcast Addr.\n- Client will use 255.255.255.255 as default.\n");
		strcpy(charBroadCastMask, "255.255.255.255"); // use 255.255.255.255 as default mask.
		//TODO: We will need to check more. E.g: Check IP format.
	}

	//
	if ((he = gethostbyname(charBroadCastMask)) == NULL) {  // get the host info
		errorExit("Error: gethostbyname");
	}

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		errorExit("Error: socket");
	}

	// allows broadcastPermission
	int broadcastPermission = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastPermission,
			sizeof broadcastPermission) == -1) {
		errorExit("Error: setsockopt");
	}

	their_addr.sin_family = AF_INET;     // host byte order
	their_addr.sin_port = htons(atoi(charPort)); // short, network byte order
	their_addr.sin_addr = *((struct in_addr *) he->h_addr);
	memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);
	if ((numbytes = sendto(sockfd, initMsg, sizeof initMsg, 0,
			(struct sockaddr *) &their_addr, sizeof their_addr)) == -1) {
		errorExit("Error: sendto");
	}

	printf("sent message \"%s\", %d bytes to %s\n", initMsg, numbytes,
			inet_ntoa(their_addr.sin_addr));
	int error, timeOut, n;
	timeOut = 300; // ms <-------------------------------------------------------inportant if you want to change timeout waiting reply
	char msg[MAX_MSG];
	/* init buffer */
	memset(msg, 0x0, MAX_MSG);
	int addrLen = sizeof(their_addr);

	//Just try to get server reply
	while (1) {

		if (isReadable(sockfd, &error, timeOut)) {
			memset(&msg, 0, sizeof msg);
			n = recvfrom(sockfd, msg, MAX_MSG, 0,
					(struct sockaddr *) &their_addr, &addrLen);
			strcpy(serverAddr, inet_ntoa(their_addr.sin_addr));
			printf("Server address is %s\n", serverAddr);
			printf("Server reply: packet contains \"%s\"\n", msg);
			printf("Server reply: \"%s\", %d bytes to %s\n", msg, n,
					inet_ntoa(their_addr.sin_addr));

			break;
		} else {
			printf("No Reply: I try again.");
			if ((numbytes = sendto(sockfd, initMsg, sizeof initMsg, 0,
					(struct sockaddr *) &their_addr, sizeof their_addr))
					== -1) {
				errorExit("Error: sendto");
			}

			printf("sent message %s, %d bytes to %s\n", initMsg, numbytes,
					inet_ntoa(their_addr.sin_addr));
		}
	}

	//TODO: I need to change Broadcast Socket to normal
	//UPDATE: to change broadcast to server ip
	//This is not to broadcast to all pc after init message from server
	//Can remove if it is not require

	if ((he = gethostbyname(serverAddr)) == NULL) {  // get the host info
		errorExit("Error: gethostbyname");
	}
	their_addr.sin_family = AF_INET;     // host byte order
	their_addr.sin_port = htons(atoi(charPort)); // short, network byte order
	their_addr.sin_addr = *((struct in_addr *) he->h_addr);
	memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);

	///UPDATE END

	//Try to get message length
	int intNumChar; //Number of chare need to request to server
	msg[0] = '\0';
	while (1) {
		if (isReadable(sockfd, &error, timeOut)) {
			memset(&msg, 0, sizeof msg);
			n = recvfrom(sockfd, msg, MAX_MSG, 0,
					(struct sockaddr *) &their_addr, &addrLen);
			printf("Server reply: Number of Char to request \"%s\"\n", msg);
			intNumChar = atoi(msg);
			break;
		} else {
			printf("No Reply: Exit.");
			if ((numbytes = sendto(sockfd, initMsg, sizeof initMsg, 0,
					(struct sockaddr *) &their_addr, sizeof their_addr))
					== -1) {
				errorExit("Error: sendto");
			}

			break;
		}
	}
	//Now start sending request and get chars

	char charSvrMsg[intNumChar]; //Store Server Message
	int j;
	int intFaultNumber = 0;
	TRY: printf("Server give me:");
	for (j = intFaultNumber; j < intNumChar; j++) {
		usleep(100);
		if ((numbytes = sendto(sockfd, reqMsg, MAX_MSG, 0,
				(struct sockaddr *) &their_addr, sizeof their_addr)) == -1) {
			errorExit("Error: sendto");
		}
		char charTemp[1];
		if (isReadable(sockfd, &error, timeOut)) {
			memset(&charTemp, 0, sizeof charTemp);
			n = recvfrom(sockfd, charTemp, sizeof charTemp, 0,
					(struct sockaddr *) &their_addr, &addrLen);
			//printf("byte :%d\n",n); //DEBUG: See number of bytes
			printf("%c", charTemp[0]);
			//Keep it
			charSvrMsg[j] = charTemp[0];
		} else {
			charSvrMsg[j] = 0;
			printf("\nNo Reply: I skip to next.\n");
		}
	}
	printf("\n");
	//Say ready to server
	printf("\nFull Sting: %s\n", charSvrMsg);
	if ((numbytes = sendto(sockfd, readyMsg, sizeof readyMsg, 0,
			(struct sockaddr *) &their_addr, sizeof their_addr)) == -1) {
		errorExit("Error: sendto");
	}
	//Now I reply server with full string
	if ((numbytes = sendto(sockfd, charSvrMsg, sizeof charSvrMsg, 0,
			(struct sockaddr *) &their_addr, sizeof their_addr)) == -1) {
		errorExit("Error: sendto");
	}

	while (1) {
		if (isReadable(sockfd, &error, timeOut)) {
			memset(&msg, 0, sizeof msg);
			n = recvfrom(sockfd, msg, MAX_MSG, 0,
					(struct sockaddr *) &their_addr, &addrLen);
			//printf("Server reply the result: \"%s\"\n", msg);

			if (strcmp(msg, okMsg) != 0) {
				//Can not get OK
				//Need to try again
				intFaultNumber = atoi(msg);
				printf("Server reply fault at %d. Need to retry\n",
						intFaultNumber + 1);
				goto TRY;
				break;
			} else {
				printf("Server reply: Ok\n");
				printf("Now, Client will exit.\n\n");
				break;
			}

			break;
		} else {
			printf("No Server approve client request\n\n");
			close(sockfd);
			return 0;
		}
	}

	close(sockfd);

	return 0;
}

/* Network Functions */
int isReadable(int sd, int * error, int timeOut) { // milliseconds
	fd_set socketReadSet;
	FD_ZERO(&socketReadSet);
	FD_SET(sd, &socketReadSet);
	struct timeval tv;
	if (timeOut) {
		tv.tv_sec = timeOut / 1000;
		tv.tv_usec = (timeOut % 1000) * 1000;
	} else {
		tv.tv_sec = 0;
		tv.tv_usec = 0;
	} // if

	if (select(sd + 1, &socketReadSet, 0, 0, &tv) == SOCKET_ERROR) {
		*error = 1;
		return 0;
	} // if
	*error = 0;

	return FD_ISSET(sd, &socketReadSet) != 0;
} /* isReadable */

/* Helper functions */
void iSleep(float milis) {
	usleep(milis * 1000);
}

void getInput(char* inp, int size_inp) {
	fflush (stdout);
	fgets(inp, size_inp, stdin);
	char *enterchar = strchr(inp, '\n'); /* search for newline character */
	if (enterchar != NULL) {
		*enterchar = '\0'; /* overwrite trailing newline */
	}
}

void errorExit(char *errorMessage) {
	printf("%s\n", errorMessage);
	perror(errorMessage);
	exit(EXIT_FAILURE); /* Exit App with error. If another app is calling this App, they know this status*/
}

void clearScreen() {
	const char* CLEAR_SCREE_ANSI = "\e[1;1H\e[2J";
	write(STDOUT_FILENO, CLEAR_SCREE_ANSI, 12);
}

void AppHeadLine() {
	printf(
			"__________________________________________________________________________\n\n");
	printf("Game Client Application \n");
	printf(
			"__________________________________________________________________________\n\n");
}