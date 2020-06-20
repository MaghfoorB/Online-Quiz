/*
MAIN SERVER:
-FIRST THREAD HANDLES CONNECTION REQUESTS FROM SUBSERVERS (SOCKET 1)
-2ND THREAD HANDLES THE EXCHANGES WITH ONE CONNECTED SUBSERVER.
-3RD THREAD HANDLES CONNECTION REQUESTS FROM CLIENTS. (SOCKET 2)
-4TH THREAD HANDLES THE EXCHANGES WITH ONE CONNECTED CLIENT.
-PARENT THREAD KEEPS AN EYE OUT FOR THE QUIT COMMAND. 
-THE QUIT COMMAND: q (then enter)
*/
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#define OUT_FILE_NAME "out.txt"
#define OWN_IP "127.1.1.1"
#define OWN_PORT 2000

char MATH_TEST_ADDR[30];
char SCI_TEST_ADDR[30];
char ENG_TEST_ADDR[30];
FILE *fp;				//different threads alter the same file, hence kept global

void getAddr(char *arr, char *g_arr)
{	//UTILITY FUNCTION: USED TO EXTRACT ONLY THE ADDRESS PART OF THE SUBSERVER STRING
	
	int i = 0, j = 0, found = 0;
	for(i=0; i<strlen(arr); i++)
	{
		if(arr[i] == ',' && !found)
		{
			found = 1;
		}
		else if(found)
		{
			g_arr[j] = arr[i];
			j++;
		}
	}
}

void* handle_client(void *args)
{	//THIS ROUTINE HANDLES COMMUNICATION WITH ONE CONNECTED CLIENT, INVOKED BY handle_clients
	
	int clientsockfd = (int)args;
	char serverMessage[100];
	char clientMessage[100];
	memset(serverMessage, '\0', sizeof(serverMessage));	
	memset(clientMessage, '\0', sizeof(clientMessage));	

	strcpy(serverMessage, "What test do you want to give, son? (MATHS, SCI, ENG)\n");
	if( send(clientsockfd, serverMessage, sizeof(serverMessage), 0) < 0)
	{
		printf("Error sending.\n");
		pthread_exit(NULL);
	}
	//WAIT FOR CLIENT'S OPTION
	printf("Waiting for client response.\n");	
	if( recv(clientsockfd, clientMessage, sizeof(clientMessage), 0) < 0)
	{	
		printf("Error recieving.\n");
		pthread_exit(NULL);
	}
	printf("%s\n", clientMessage);	
	//SEND ADDRESS OF APPROPRIATE SUBSERVER BACK TO CLIENT
	if(clientMessage[0] == 'M')
	{
		if( send(clientsockfd, MATH_TEST_ADDR, sizeof(MATH_TEST_ADDR), 0) < 0)
		{	
			printf("Could not send address.\n");
			pthread_exit(NULL);
		}
	}
	else if(clientMessage[0] == 'S')
	{
		if( send(clientsockfd, SCI_TEST_ADDR, sizeof(SCI_TEST_ADDR), 0) < 0)
		{	
			printf("Could not send address.\n");
			pthread_exit(NULL);
		}
	}
	else if(clientMessage[0] == 'E')
	{
		if( send(clientsockfd, ENG_TEST_ADDR, sizeof(ENG_TEST_ADDR), 0) < 0)
		{	
			printf("Could not send address.\n");
			pthread_exit(NULL);
		}
	}

	close(clientsockfd);
	pthread_exit(NULL);
}


void* handle_subserver(void *args)
{	//THIS ROUTINE HANDLES COMMUNICATION WITH ONE CONNECTED SUBSERVER, INVOKED BY handle_servers
	
	int subfd = (int)args;
	char test = '\0';
	char subMessage[100];
	memset(subMessage, '\0', sizeof(subMessage));
	
	if(recv(subfd, subMessage, sizeof(subMessage), 0) < 0)
	{
		printf("Error receiving message.\n");
		pthread_exit(NULL);	//or return
	}
	printf("SubServer: %s\n", subMessage);	//the subserver string
	
	/*assign ip and port in this message to appropriate global variable based on test type
	handle_client thread will pick up address of the test it wants from there
	*/
	
	test = subMessage[0];
	if(test == 'M')
	{
		getAddr(subMessage, MATH_TEST_ADDR);
	}
	else if(test == 'S')
	{
		getAddr(subMessage, SCI_TEST_ADDR);
	}
	else if(test == 'E')
	{
		getAddr(subMessage, ENG_TEST_ADDR);
	}
	
	//WAIT FOR THE FINAL RESULT OF THE TEST BEING CARRIED OUT
	
	while(1)
	{
		memset(subMessage, '\0', sizeof(subMessage));
		int r = recv(subfd, subMessage, sizeof(subMessage), 0);
		if(r < 0)
		{
			printf("Error receiving message.\n");
			break;
		}
		else if (r == 0)
		{
			printf("Subserver disconnected.\n");
			break;
		}

		printf("%s\n", subMessage);
		//FILE IN THE MESSAGE
		fprintf(fp, "%s\n", subMessage);	//file is already open
	}
	
	//CLEARING ADDRESS VARIABLES SO THAT CLIENT DOESNT CONNECT TO AN OFFLINE SERVER
	if(test == 'M')
	{
		memset(MATH_TEST_ADDR, '\0', sizeof(MATH_TEST_ADDR));			
	}
	else if(test == 'S')
	{
		memset(SCI_TEST_ADDR, '\0', sizeof(SCI_TEST_ADDR));
	}
	else if(test == 'E')
	{
		memset(ENG_TEST_ADDR, '\0', sizeof(ENG_TEST_ADDR));
	}
	
	close(subfd);
	pthread_exit(NULL);
}


void* handle_subservers(void *args)
{	//THIS ROUTINE RUNS FOREVER AND LISTENS FOR INCOMING SUBSERVER CONNECTIONS ON PORT 2001, INVOKED BY PARENT THREAD
	
	int serverfd2;
	pthread_t thread2;
	struct sockaddr_in serverAdd2, subAdd_client;	//serverAdd2 is for the 2nd socket of the server.c application
	
	//CREATING AND BINDING THE 2ND SOCKET OF THE PROGRAM
	serverfd2 = socket(AF_INET, SOCK_STREAM, 0);
	if(serverfd2 < 0)
	{
		printf("Could not create subserver socket.\n");
		pthread_exit(NULL);	//or return
	}
	serverAdd2.sin_family = AF_INET;		
    serverAdd2.sin_port = htons(2001);					//different port
    serverAdd2.sin_addr.s_addr = inet_addr(OWN_IP);    //same ip as 'serverfd'
	if( bind(serverfd2, (const struct sockaddr*)&serverAdd2, sizeof(serverAdd2)) < 0)
	{
		printf("Could not bind subserver socket.\n");
		pthread_exit(NULL);
	}
	
	while(1)
	{
		listen(serverfd2, 3);
		printf("Listening for servers...\n");
		int subAddsize = sizeof(subAdd_client);
    	int subfd = accept(serverfd2, (struct sockaddr*)&subAdd_client, (socklen_t*)&subAddsize);
    	printf("Sub server %d accepted. Assigned thread.\n", subfd);
    	
    	/*HANDLE EXCHANGES WITH ONE CONNECTED SUBSERVER IN ANOTHER THREAD
    	THIS WILL BE THE 2ND CHILD THREAD IN THE PROGRAM
    	*/
    	int t2 = pthread_create(&thread2,NULL,handle_subserver,(void*)subfd);
    	
	}
	pthread_exit(NULL);
}


void* handle_clients(void* args)
{	//THIS ROUTINE RUNS FOREVER AND LISTENS FOR CLIENT CONNECTIONS, INVOKED BY PARENT THERAD

	int serverfd;		//for client connections
    char serverMessage[100];
    pthread_t thread4;
    struct sockaddr_in serverAdd, clientAdd;
	memset(serverMessage, '\0', sizeof(serverMessage));	
	
	//CREATE AND BIND THE SOCKET FOR CLIENTS
    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if(serverfd < 0)
    {
    	printf("Could not create socket.\n");
    	pthread_exit(NULL);
    }
    serverAdd.sin_family = AF_INET;
    serverAdd.sin_port = htons(OWN_PORT);
    serverAdd.sin_addr.s_addr = inet_addr(OWN_IP);   
    if(bind(serverfd, (const struct sockaddr*)&serverAdd, sizeof(serverAdd)) < 0)
    {
    	printf("Could not bind.\n");
    	pthread_exit(NULL);
    }
	
	while(1)
	{
		listen(serverfd, 3);
    	printf("Listening for clients....\n");
    	int clientAddsize = sizeof(clientAdd);
    	int clientsockfd = accept(serverfd, (struct sockaddr*)&clientAdd, (socklen_t*)&clientAddsize);
    	printf("Connected to client: %d\n", clientsockfd);	
		
		/*HANDLE THE EXCHANGES WITH ONE CONNECTED CLIENT IN ANOTHER THREAD
		THIS WILL BE THE 4TH CHILD THREAD IN THE PROGRAM
		*/
		
		int t4 = pthread_create(&thread4,NULL,handle_client,(void*)clientsockfd);
	}
	
	close(serverfd);
	pthread_exit(NULL);
}


int main()
{
    char input[2];
    memset(input, '\0', sizeof(input));
    pthread_t thread1, thread3;	
	//INITIALIZING THE GLOBAL VARIABLES
	memset(MATH_TEST_ADDR, '\0', sizeof(MATH_TEST_ADDR));
	memset(SCI_TEST_ADDR, '\0', sizeof(SCI_TEST_ADDR));
	memset(ENG_TEST_ADDR, '\0', sizeof(ENG_TEST_ADDR));
	fp = fopen(OUT_FILE_NAME, "w+");
	
	//START LISTENING FOR INCOMING SUBSERVER CONNECTIONS ON ANOTHER SOCKET
	int t1 = pthread_create(&thread1, NULL, handle_subservers, NULL);	//subserver requests are now being handled by this thread
	int t3 = pthread_create(&thread3, NULL, handle_clients, NULL);		//client requestts are now bieng handled by this thread
		
	printf("Input waiting..\n");
	gets(input);
	if(input[0] == 'q')
	{
		close(fp);
		printf("Exited\n");
    	return 0;
    }
}

