/*****************************************************************
**File Description: Server for decryption
**Takes a port for parameters
**Author: Jennifer Hackworth
**Date: 5/25/2016
******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#define MAX_SIZE 100000

void getFile(int sock, char *file);
char *decryptMessage(char *encrypt, char *key);
void sendFile(int sock, char *file);

int main(int argc, char *argv[])
{
	int connectNo = 0, sockfd, newsockfd, portno, pid, n;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	char key[MAX_SIZE], txtFile[MAX_SIZE];
	char *messageFile = malloc(sizeof(char) * MAX_SIZE);

	/*Checks for correct number of parameters*/
	if(argc < 2)
	{
		printf("ERROR: no port provided.\n");
		exit(1);
	}

	/*sets up socket and port info*/
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if(sockfd < 0)
		printf("ERROR opening socket\n");

	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("Error on binding\n");
		exit(1);
	}

	listen(sockfd, 5);
	clilen = sizeof(cli_addr);

	/*loop to accepts new connections and then forks*/
	while(connectNo < 10)
	{
		/*accepts new connection and then creates a child process*/
		newsockfd = accept(sockfd,  (struct sockaddr *) &cli_addr, &clilen);
		if(newsockfd < 0)
			printf("Error on accept\n");

		pid = fork();
		if(pid < 0)
			printf("Error on fork\n");

		
		if(pid == 0)
		{
			
			close(sockfd);
			/*checks to make sure server is communicating with correct client*/
			n = write(newsockfd, "otp_dec_d", 9);

			/*recieves both files then decrypts message then sends message to client*/
			getFile(newsockfd, txtFile);
			getFile(newsockfd, key);
			memset(messageFile, 0, MAX_SIZE);
		
			messageFile = decryptMessage(txtFile, key);
			sendFile(newsockfd, messageFile);	
			exit(0);
		}

	close(newsockfd);
	connectNo++;
	}
	close(sockfd);
	return 0;
}

/* 
**	recieves file from client
**	parameters: file and socket
**	returns: none
*/
void getFile(int sock, char *file)
{
	int recv_bytes, total_bytes = 0;
	char buffer[MAX_SIZE];
	char delimiter = '\0';

	/*loops through until null char is encountered*/
	do
	{	
	
		recv_bytes = recv(sock, file, MAX_SIZE, 0);
		if(recv_bytes < 0)		
		{	
			recv_bytes = 0;
			/*printf("recv error\n");
			close(sock);
			exit(1);*/
		}

		total_bytes += recv_bytes;
	}while(file[total_bytes] != delimiter);

	/*sends confirmation of recieved file to client*/
	recv_bytes = write(sock, "got file", 8);
}

/* 
**	decrypts message
**	parameters: encrypted file and key
**	returns: decrypted message
*/
char *decryptMessage(char *encrypt, char *key)
{
	int messageNum, encryptNum, keyNum, i, count;
	
	/*finds length of text to decrypt*/
	int cipherLen = strlen(encrypt);	
	
	char *message = malloc(sizeof(char) *cipherLen);

	/*loops through chars in decrypt file until new line is reached*/
	for(i = 0; encrypt[i] != '\n'; i++)
	{
		/*if a letter is a space it is assigned the number 27*/
		if(key[i] == ' ')
			keyNum = 27;
		
		/*or else it is assigned a number based of it's position*/
		else
			keyNum = key[i]  - 64;

		if(encrypt[i] == ' ')
			encryptNum = 27;
	
		else
			encryptNum = encrypt[i] - 64;
		
		/*subtract the encyptedNum by the key, add 1 because that is how I got it to work*/
		messageNum = (encryptNum - keyNum) + 1;
	
		/*If the number is negative, loops back though the alphabet*/
		if(messageNum < 0)
			messageNum += 27;

		/*if it is greater than 26, assign it the space*/
		if(messageNum >= 26)
			message[i] = ' ';

		else
			message[i] = (messageNum) + 'A';
	}

	return message;
}	

/* 
**	send file to client
**	parameters: socket and filename
**	returns: none
*/
void sendFile(int sock, char *file)
{
	int send_bytes, send_count, send_file_size;


	send_count = 0;

	send_file_size = strlen(file);

	/*loops though until complete file is sent*/
	while(send_count < send_file_size)
	{
		send_bytes = send(sock, file, (send_file_size - send_count), 0);
		if(send_bytes < 0)
		{
			send_bytes = 0;
			/*printf("Error sending file \n");
			exit(1);*/
		}
	send_count += send_bytes;
	}
} 

