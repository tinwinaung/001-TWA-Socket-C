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
#include <time.h>
#include <sys/time.h>
#include <math.h>
/* Definations */
#define EXIT_SUCCESS	0
#define EXIT_FAILURE	1
#define MAXBUFLEN 		100
#define SOCKET_ERROR -1
#define N_MAX 10000000 //For rand
/* Network */
void *get_in_addr(struct sockaddr *sa);
/* Helper */
char * randStr(int m);
void getInput(char* inp, int size_inp);
void errorExit(char *errorMessage);
void clearScreen();
void AppHeadLine();
void iSleep(float milis);
double getUniRand();

int main(void) {
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv, j;
	struct timeval tv1, tv2, tS1, tS2;
	double tdiff, lamd;
	float xval;
	double bigTval;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	char charPort[50]; //store user input for port
	char charT[50];
	char *charSvrMessage;
	char okMsg[] = "OK";
	char initMsg[] = "init"; // Broadcast message in this protocol
	char readyMsg[] = "ready"; // Server reply ready to serve
	char reqMsg[] = "req"; // Client request
	int sw = 0; //switch for time record
	double activetime; //time to measure serve time
	int srvtime; 
	srand(time(0)); //Seed the pseudo-random number generator
//	xval = getUniRand(); //Uncomment this if you want only one xvalue per session
	clearScreen();
	AppHeadLine();
	//Ask port number for server

//	return 0;
	//Update show x value here
	//printf("Server is using x value = %f\n", xval);
	//Asking x value if want to modify - no input use rand value

//	printf("Please enter xValue\nPress enter (do not put value) to use Rand value\nxVal[%f]:",xval);
//	getInput(charPort, sizeof charPort);
//	if (strlen(charPort) == 0) {
//		printf(
//				"- Server will use xVale=%f as default.\n",xval);
//	}else{
//		xval = atof(charPort);
//	}

	//init serve time
	srvtime=10;
	printf("How long do you want to serve client after receiving broadcast packet.\nServe time - Seconds [%d]:",srvtime);
	getInput(charPort, sizeof charPort);
	if (strlen(charPort) == 0) {
		printf(
				"- Server will usedefault serve time %d Seconds.\n",srvtime);
	}else{
		srvtime = atof(charPort);
	}
	//Clear previous input
	memset(&charPort, 0, sizeof(charPort));
	printf("Please enter port number to listen\nPort[9999]:");
	getInput(charPort, sizeof charPort);
	if (strlen(charPort) == 0) {
		printf(
				"Server can not start without port.\n- Server will use 9999 as default.\n");
		strcpy(charPort, "9999"); // use 9999 as default port. Assign it to port since user have no input
		//TODO: We will need to check more. E.g: check integer.
	}
	//
	printf(
			"Please enter Big T Value \nEnter 0 if you dont want to use drop formula\nmicro sec[10]:");
	getInput(charT, sizeof charT);
	if (strlen(charT) == 0) {
		printf("Server need big T value.\n- Server will use 80 as default.\n");
		strcpy(charT, "10"); // use 9999 as default port. Assign it to port since user have no input
		//TODO: We will need to check more. E.g: check integer.
	}
	bigTval = atof(charT);

	charSvrMessage = randStr(30);
	printf("Server secret message is %s\n", charSvrMessage);
	printf("Server is using x value = %f\n", xval);
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM; // UDP
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, charPort, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); // Special failure :P
		return (EXIT_FAILURE);

	}
	printf(
			"\nNow Server is listening on Port %s \nPlease Broacast your \"init\" message for me.\n",
			charPort);

	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol))
				== -1) {
			errorExit("listener: socket");
			continue;
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		errorExit("listener: failed to bind socket\n");
	}

	freeaddrinfo(servinfo);
	StartWaitClient: printf(
			"Server is waiting client \"init\" Broadcast message...\n");
	
	//clear all buffers
	addr_len = sizeof their_addr;

	//Now waiting for Broadcast
	while (1) {
		gettimeofday(&tv1, NULL);
		sw = 0;
		memset(&buf, 0, sizeof(buf));
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
				(struct sockaddr *) &their_addr, &addr_len)) == -1) {
			errorExit("Error: recvfrom");
		}

		if (strcmp(buf, initMsg) != 0) {
			printf("Server: got Broadcast messag from %s\n",
					inet_ntop(their_addr.ss_family,
							get_in_addr((struct sockaddr *) &their_addr), s,
							sizeof s));
			printf(
					"But it is not define in protocol.\nServer is waiting for next Broadcast\nThe message is : %s\n",
					buf);
		} else {
			gettimeofday(&tS1, NULL);
			printf("Server: got \"init\" from %s\n",
					inet_ntop(their_addr.ss_family,
							get_in_addr((struct sockaddr *) &their_addr), s,
							sizeof s));
			printf("Server reply: ready\n");
			//Here we are reply
			if (sendto(sockfd, readyMsg, sizeof readyMsg, 0,
					(struct sockaddr *) &their_addr, addr_len) == -1) {
				errorExit("Error: sendto(ready)");
			}
			break;
		}

	}
	//Now, we reply message length
	//Becareful, I try to conver int to char here
	int intSvrMsgLen = strlen(charSvrMessage);
	char chrMsgLength[100];
	sprintf(chrMsgLength , "%d", intSvrMsgLen);

	if (sendto(sockfd, chrMsgLength, sizeof chrMsgLength, 0,
			(struct sockaddr *) &their_addr, addr_len) == -1) {
		errorExit("Error: sendto(message umber)");
	}
	//Now, try to listen request to send char.
	int k = 0;
	
	REARM: printf("Server send char to Client:\n");
	for (j = k; j < intSvrMsgLen; j++) {
		//Check active interval

		gettimeofday(&tS2, NULL);
		activetime = (double) (tS2.tv_usec - tS1.tv_usec) / 1000000
				+ (double) (tS2.tv_sec - tS1.tv_sec);
		//printf("Active %f\n",activetime);
		if (activetime > srvtime) {

			printf("Serve time is exceed to %d seconds.\n\n\n",srvtime);
			goto StartWaitClient;
		}
		//
		memset(&buf, 0, sizeof(buf));
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
				(struct sockaddr *) &their_addr, &addr_len)) == -1) {
			errorExit("Error: recvfrom");
		}
		//UPDATE : Previous logic is wrong
		if (sw == 0) {
			gettimeofday(&tv2, NULL);
			sw = 1;
		} else {
			gettimeofday(&tv1, NULL);
			sw = 0;
		}
		//--------------------------------------out is Sec
		tdiff = (double) (tv2.tv_usec - tv1.tv_usec) / 1000000
				+ (double) (tv2.tv_sec - tv1.tv_sec);
		if (tdiff < 0) {
			tdiff = tdiff * (-1);
		}
		tdiff = tdiff * 1000000;
		//gettimeofday(&tv1, NULL);
		printf(" td: %f ", tdiff);
		if (strcmp(buf, reqMsg) != 0) {
			errorExit("Error: Invalid Client request");
		} else {
			char charTemp[1];
			charTemp[0] = charSvrMessage[j];
			printf("%c", charTemp[0]);
			//Here we  reply
			lamd = tdiff - bigTval;
			lamd = lamd * (-1);
			lamd = exp(lamd);
			if (lamd > 1) {
				lamd = 1;
			}
			printf(" lamd: %f", lamd);
			xval = getUniRand();
			printf(" x: %f\n", xval);
			if (!(lamd > xval) || bigTval == 0) {
				if (sendto(sockfd, charTemp, sizeof charTemp, 0,
						(struct sockaddr *) &their_addr, addr_len) == -1) {
					errorExit("Error: sendto(Char)");
				}
			}
		}
	}
	printf("Finish string.");
	printf("\n");

	memset(&buf, 0, sizeof(buf));
	if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
			(struct sockaddr *) &their_addr, &addr_len)) == -1) {
		errorExit("Error: recvfrom");
	}
	//printf("%s\n", buf);

	if (strcmp(buf, readyMsg) != 0) {
		printf("Error: Invalid Client request2\n%d\n%s\n%s",numbytes,buf,readyMsg);
		errorExit("Error: Invalid Client request2");
	} else {
		memset(&buf, 0, sizeof(buf));
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN - 1, 0,
				(struct sockaddr *) &their_addr, &addr_len)) == -1) {
			errorExit("Error: recvfrom");
		}
		//Now we compare
		for (j = 0; j < intSvrMsgLen; j++) {
			if (charSvrMessage[j] == buf[j]) {

			} else {
				memset(&chrMsgLength, 0, sizeof(chrMsgLength));
				sprintf(chrMsgLength, "%d", j);
				printf("fault found at : %d\n", j + 1);
				//Send it back to client
				if (sendto(sockfd, chrMsgLength, sizeof chrMsgLength, 0,
						(struct sockaddr *) &their_addr, addr_len) == -1) {
					errorExit("Error: sendto()");
				}
				k = j;
				goto REARM;
				break;
			}
		}

		if (sendto(sockfd, okMsg, sizeof okMsg, 0,
				(struct sockaddr *) &their_addr, addr_len) == -1) {
			errorExit("Error: sendto()");
		}
		printf("\nClient successfully provide correct string.\nNow Server will exit.\n\n");

	}

	close(sockfd);

	return 0;
}

/* Network Functions */
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
if (sa->sa_family == AF_INET) {
	return &(((struct sockaddr_in*) sa)->sin_addr);
}
return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

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
char * randStr(int m) {
int i, s = m; //int i,s=1 + rand()%m;
char t;
char * word = (char *) malloc(s);
for (i = 0; i < s; ++i) {
	t = rand() % 52;
	word[i] = 'A' + (t >> 1) + ((1 & t) << 5);
}
word[i] = '\0';
return word;
}
void iSleep(float milis) {
usleep(milis * 100);
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
printf("Game Server Application\n");
printf(
		"__________________________________________________________________________\n\n");
}

double getUniRand() {
double x;
x = (double) rand() / (double) RAND_MAX;
return x;
//	int i;
//	double sum_0, sum_1, mean, var, x;
//	for (i = 1, sum_0 = 0., sum_1 = 0.; i <= N_MAX; i++) {	
//		sum_0 += x;
//		sum_1 += (x - 0.5) * (x - 0.5);
//	}
//	mean = sum_0 / (double) N_MAX;
//	var = sum_1 / (double) N_MAX;
}