#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

#define SERVER_PORT 2000
#define SERVER_IP "127.1.1.1"

void getAddr(char *arr, char *ip, char *port)
{	//UTILITY FUNCTION: EXTRACTS IP AND PORT OF SUBSERVER FROM THE MESSAGE SENT BY THE MAIN SERVER
	
	int i = 0, j = 0, k = 0, found = 0;
	for(i=0; i<strlen(arr); i++)
	{
		if(arr[i] == ',')
		{
			found = 1;
		}
		else if(found)		//first comma encountered, now port no. starts
		{
			port[j] = arr[i];
			j++;
		}
		else if(!found)		//everything before the first comma is the ip
		{
			ip[k] = arr[i];
			k++;
		}
	}
}

int main()
{
    int sockfd;
    struct sockaddr_in serverAdd, subserverAdd;
	char serverMessage[100];
	char clientMessage[100];	
	memset(serverMessage, '\0', sizeof(serverMessage));
	memset(clientMessage, '\0', sizeof(clientMessage));

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    serverAdd.sin_family = AF_INET;
    serverAdd.sin_port = htons(SERVER_PORT);
    serverAdd.sin_addr.s_addr = inet_addr(SERVER_IP);

    if(connect(sockfd, (struct sockaddr*)&serverAdd, sizeof(serverAdd)) < 0)
	{
		printf("Failed to connect to server.\n");
		return -1;
	}
	else printf("Connected to Main Server.\n");

	//RECIEVE THE INITIAL MESSAGE FROM THE SERVER
	if(recv(sockfd, serverMessage, sizeof(serverMessage), 0) < 0)
	{
		printf("Error receiving message.\n");
		return -1;
	}
	else printf("Server says: %s", serverMessage);
	
	//SEND SELECTED OPTION
	gets(clientMessage);
	if(send(sockfd, clientMessage, sizeof(clientMessage), 0) < 0)
	{
		printf("Error sending message.\n");
		return -1;
	}
	else printf("Message sent.\n");	

	//RECIEVE ADDRESS OF SUBSERVER
	memset(serverMessage, '\0', sizeof(serverMessage));
	if(recv(sockfd, serverMessage, sizeof(serverMessage), 0) < 0)
	{
		printf("Could not recieve address.\n");
		return -1;
	}

	//EXTRACT PORT AND IP FROM MESSAGE
	int sub_server_port = 0;
	char sub_server_ip[20], p[5];
	memset(sub_server_ip, '\0', sizeof(sub_server_ip));
	memset(p, '\0', sizeof(p));
	getAddr(serverMessage, sub_server_ip, p);
	sub_server_port = atoi(p);
	
	printf("IP of subserver: %s\n", sub_server_ip);
	printf("PORT of subserver: %d\n", sub_server_port);
	
	//NOW CONNECT TO THE SUBSERVER
	close(sockfd);		//first close before reusing for a different address.
	sockfd = socket(AF_INET, SOCK_STREAM, 0);	
	subserverAdd.sin_family = AF_INET;
    subserverAdd.sin_port = htons(sub_server_port);
    subserverAdd.sin_addr.s_addr = inet_addr(sub_server_ip);	
	
	if(connect(sockfd, (struct sockaddr*)&subserverAdd, sizeof(subserverAdd)) < 0)
	{
		printf("Failed to connect to sub-server.\n");
		printf("Error!: %s (%d)\n", strerror(errno), errno);
		return -1;
	}
	else printf("Connected to Sub-server.\n");
	
	//CONNECTED: NOW RECIEVE THE FIRST MESSAGE FROM THE SUBSERVER AND PRINT
	memset(serverMessage, '\0', sizeof(serverMessage));
	memset(clientMessage, '\0', sizeof(clientMessage));
	if(recv(sockfd, serverMessage, sizeof(serverMessage), 0) < 0)
	{
		printf("Error receiving message.\n");
		return -1;
	}
	printf("%s\n", serverMessage);
	gets(clientMessage);		//validate
	
	//SEND USER'S REPLY BACK TO THE SUBSERVER
	if(send(sockfd, clientMessage, sizeof(clientMessage), 0) < 0)
	{
		printf("Error sending message to subserver.\n");
		return -1;
	}
	
	//RECIEVE THE SCORE
	memset(serverMessage, '\0', sizeof(serverMessage));
	if(recv(sockfd, serverMessage, sizeof(serverMessage), 0) < 0)
	{
		printf("Error receiving message.\n");
		return -1;
	}
	printf("%s\n", serverMessage);
	
		
	memset(serverMessage, '\0', sizeof(serverMessage));
	memset(clientMessage, '\0', sizeof(clientMessage));
	close(sockfd);
    return 0;
}
