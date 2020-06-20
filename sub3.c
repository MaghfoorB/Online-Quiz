/*
SUB-SERVER 3: HANDLES ENGLISH TESTS
-RUNS TWO CHILD THREADS.
-ONE FOR LISTENING TO CLIENT REQUESTS AND ONE FOR MAINTAINING CONNECTION WITH THE MAIN SERVER
-BOTH THREADS HAVE THEIR OWN SOCKETS.
-PARENT THREAD KEEPS AN EYE FOR THE QUIT COMMAND.
-THE QUIT COMMAND: q (then enter)
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#define TEST_TYPE "ENG"
#define MAIN_SERVER_IP "127.1.1.1"
#define MAIN_SERVER_PORT 2001
#define OWN_IP "127.1.1.1"	//this sub server is listening for client connections on these ip and port
#define OWN_PORT 2005

const char* makeMessage(char *message)
{	//UTILITY FUNCTION
	
	strcat(message, (const char*)&TEST_TYPE);
	strcat(message, ",");
	strcat(message, (const char*)&OWN_IP);
	strcat(message, ",");
	char p[6];						
	memset(p, '\0', sizeof(p));			//converting port number to string type
	sprintf(p,"%ld", OWN_PORT);
	strcat(message, (const char*)&p);	
	return message;
} 

struct args
{
   int port, sockfd, client_sockfd;
   char ip[16];
};

void* handle_client(void* argument)
{	//THIS ROUTINE HANDLES COMMUNICATION WITH ONE CONNECTED CLIENT, INVOKED BY PARENT THREAD
	
	struct args a = *((struct args*)argument);
	int client_sockfd = a.client_sockfd;
	char myMessage[100], clientMessage[100];
	memset(myMessage, '\0', sizeof(myMessage));
	memset(clientMessage, '\0', sizeof(clientMessage));
	
	strcpy(myMessage, "Select a test:\n1) Analogies\n2) Antonyms\n3) RC\n");
	if( send(client_sockfd, myMessage, sizeof(myMessage), 0) < 0)
	{
		printf("Sub server could not send the message.\n");
		pthread_exit(NULL);
	}
	
	//RECIEVE TYPE FROM CLIENT
	if(recv(client_sockfd, clientMessage, sizeof(clientMessage), 0) < 0)
	{
		printf("Error receiving message from client.\n");
		pthread_exit(NULL);
	}
	
	//SEND BACK TEST RECORD AND A RANDOM SCORE
	int score = 0;
	char type[9];
	srand(time(NULL));	//seed for rand()
	memset(myMessage, '\0', sizeof(myMessage));
	memset(type, '\0', sizeof(type));
	
	if(clientMessage[0] == '1')
	{	
		strcpy(type, "Analogies");	
	}
	else if(clientMessage[0] == '2')
	{
		strcpy(type, "Antonyms");
	}
	else if(clientMessage[0] == '3')
	{
		strcpy(type, "RC");
	}
	char str_score[3];
	score = rand()%100;
	sprintf(str_score, "%d", score);	//integer to string
	strcpy(myMessage, "Name:\t");
	strcat(myMessage, TEST_TYPE);
	strcat(myMessage, "\nType:\t");
	strcat(myMessage, type);
	strcat(myMessage, "\nScore:\t");
	strcat(myMessage, str_score);
	strcat(myMessage, "/100\nGoodbye :)\n");
	//FIRST SEND THE INFO TO THE CLIENT
	if( send(client_sockfd, myMessage, sizeof(myMessage), 0) < 0)
	{	
		printf("Could not send the score.\n");
		pthread_exit(NULL);
	}
	
	//THEN SEND TO THE MAIN SERVER (in a different format)
	memset(myMessage, '\0', sizeof(myMessage));
	strcpy(myMessage, "-");
	strcat(myMessage, a.ip);
	strcat(myMessage, ", ");
	char p[6];						
	memset(p, '\0', sizeof(p));
	sprintf(p, "%d", a.port);		//converting port no to string for concatenation
	strcat(myMessage, p);
	strcat(myMessage, ", ");
	strcat(myMessage, TEST_TYPE);
	strcat(myMessage, ", ");
	strcat(myMessage, type);
	strcat(myMessage, ", ");
	strcat(myMessage, str_score);
	strcat(myMessage, "\n");
	
	if( send(a.sockfd, myMessage, sizeof(myMessage), 0) < 0)
	{
		printf("Could not send to main server.\n");
		pthread_exit(NULL);
	}
	
	printf("Client session ended.\n");
	pthread_exit(NULL);	
}

void* handle_clients(void* args)
{	//THIS ROUTINE RUNS FOREVER AND LISTENS TO INCOMING CLIENT CONNECTIONS ON ANOTHER PORT	

	int sockfd = (int)args;
	int sockfd2;	//the second socket of the program
	struct sockaddr_in myAddr, clientAdd;
	pthread_t thread2;

	sockfd2 = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd2 < 0)
	{
		printf("Could not create client socket.\n");
		pthread_exit(NULL);
	}
	myAddr.sin_family = AF_INET;
	myAddr.sin_port = htons(OWN_PORT);
	myAddr.sin_addr.s_addr = inet_addr(OWN_IP);
	if(bind(sockfd2, (const struct sockaddr*)&myAddr, sizeof(myAddr)) < 0)
    {
    	printf("Could not bind.\n");
    	pthread_exit(NULL);
    }
    //LISTENING FOR CLIENT CONNECTIONS
    printf("Sub-Server for %s is now listening for client connections...\n", TEST_TYPE);
    int clientAddsize, clientsockfd;
    while(1)
    {
    	listen(sockfd2, 3);
    	clientAddsize = sizeof(clientAdd);
    	clientsockfd = accept(sockfd2, (struct sockaddr*)&clientAdd, (socklen_t*)&clientAddsize);
    	printf("Client connected with fd: %d\n", clientsockfd);
    	//PASS CLIENT'S SOCKET FD, IP, PORT AND THE FIRST SOCKET OF THIS SUBSERVER TO THREAD
    	//FOR FURTHER PROCESSING
    	struct args a;
    	a.sockfd = sockfd;
    	a.client_sockfd = clientsockfd;
    	a.port = ntohs(clientAdd.sin_port);
    	memset(a.ip, '\0', sizeof(a.ip));
    	strcpy(a.ip, inet_ntoa(clientAdd.sin_addr));
    	
    	//ALL EXCHANGES WITH THE CLIENT ARE NOW HANDLED BY THE THREAD BELOW
    	//THIS IS THE 2ND CHILD THREAD OF THE PROGRAM
    	int t2 = pthread_create(&thread2,NULL,handle_client,(void*)&a);	
	}
	
	close(sockfd2);
	pthread_exit(NULL);
}


int main()
{
	int sockfd;					//sockfd is for server and sockfd2 (in hanlde_clients) is for user connections
								//sockfd behaves as a client socket, sockfd2 behaves as a server socket
	struct sockaddr_in serverAdd;
	char message[100];
	char input[2];
	memset(input, '\0', sizeof(input));
	memset(message, '\0', sizeof(message));
	pthread_t thread1;	
	
	//CREATING SOCKET	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
    	printf("Could not create socket.\n");
    	return -1;
    }
    //SETTING ATTRIBUTES OF SERVER
    serverAdd.sin_family = AF_INET;
    serverAdd.sin_port = htons(MAIN_SERVER_PORT);		//subserver requests are being handled on port 2001
    serverAdd.sin_addr.s_addr = inet_addr(MAIN_SERVER_IP);

	if(connect(sockfd, (struct sockaddr*)&serverAdd, sizeof(serverAdd)) < 0)
	{
		printf("Failed to connect to server.\n");
		return -1;
	}
	else printf("Connected to Main Server.\n");
	
	makeMessage(message);	//subserver's own ip, port etc
	
	//SEND MESSAGE TO SERVER
	if(send(sockfd, message, sizeof(message), 0) < 0)	
	{
		printf("Error sending message.\n");
		return -1;
	}

	//CONNECTED TO MAIN SERVER. NOW START LISTENING FOR CLIENT REQUESTS
	int t1 = pthread_create(&thread1, NULL, handle_clients, (void*)sockfd);
	
	gets(input);
	if(input[0] == 'q')
	{
		close(sockfd);
		return 0;
	}
}
