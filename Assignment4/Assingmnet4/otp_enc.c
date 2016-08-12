/*****************************************************************
**File Description: Client for encryption
**Takes 2 files and a port for parameters, returns encrypted file.
**Author: Jennifer Hackworth
**Date: 5/25/2016
******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>

#define MAX_SIZE 100000

int validateFilesLength(char *text, char *key);
int validateFile(char *file, int length);
void sendFile(int sock, char *file);
void getFile(int sock, char *file);

int main(int argc, char *argv[])
{
	int sockfd, portno, fileSize, errorFlag, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[MAX_SIZE];

	/*Checks the number of arguments entered*/
	if(argc < 4)
	{
		printf("Error: Incorrect number of arguments\n");
		exit(1);
	}

	/*sets up port and connection to socket*/
	portno = atoi(argv[3]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		printf("Error opening socket.\n");
		exit(1);
	}
	server = gethostbyname("localhost");

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("ERROR connecting\n");
		close(sockfd);
		exit(1);
	}

	/*checks to see if the key length is longer than the file length*/
	fileSize = validateFilesLength(argv[1], argv[2]);
	if(fileSize < 0)
	{
		printf("Error: Key file is smaller than text file.\n");
		close(sockfd);
		exit(1);
	}

	/*checks to make sure connected to correct server*/
	memset(buffer, 0, sizeof(buffer));
	n = read(sockfd, buffer, 9);

	if(strcmp(buffer, "otp_enc_d") != 0)
	{
		printf("ERROR: Client is attempting to connect to wrong server\n");
		exit(1);
	}

	/*sends first file then waits for recieve confirmation before sending next*/
	sendFile(sockfd, argv[1]);

	memset(buffer, 0, sizeof(buffer));
	n = read(sockfd, buffer, MAX_SIZE);

	/*sends 2nd file then waits for recieve confirmation*/
	sendFile(sockfd, argv[2]);

	memset(buffer, 0, sizeof(buffer));
	n = read(sockfd, buffer, MAX_SIZE);

	/*recieves and prints decrypted message*/
	memset(buffer, 0, sizeof(buffer));
	getFile(sockfd, buffer);

	printf("%s\n", buffer);

	close(sockfd);

	return 0;
}

/* 
**	checks to ensure that they key is longer than the file
**	parameters: decrypted message file and key file
**	returns: int that confirms length
*/
int validateFilesLength(char *text, char *key)
{
	/*
	 *uses the stat struct so file 
	 *does not need to be opened to be checked
	 */
	struct stat txt, ky;
	stat(text, &txt);
	stat(key, &ky);

	/*
	 *compares size and returns the size 
     *if key is longer, -1 if it is shorter
	 */
	if(txt.st_size > ky.st_size)
		return -1;

	else
		return ky.st_size;
}

/* 
**	checks to ensure that the file only contains capital
**  letters and spaces
**	parameters: file and file length
**	returns: 1 if valid, 0 if not
*/
int validateFile(char *file, int length)
{
	int i;
	for(i = 0; i < length; i++)
	{
		if(file[i] != 32 && file[i] != 10 && !(file[i] >= 65 && file[i] <= 90))
			return -1;
	}

	return 1;
}
	
/* 
**	opens and send file to server
**	parameters: socket and filename
**	returns: none
*/		
void sendFile(int sock, char *file)
{
	int valid, fd, send_bytes, send_count, send_file_size;
	char buffer[MAX_SIZE];
	char delimiter = '\n';
	memset(buffer, 0, MAX_SIZE);

	send_count = 0;

	/*opens file*/
	FILE *fp = fopen(file, "r");
	if(fp < 0)
	{
		printf("Error opening file\n");
		close(sock);
		exit(1);
	}

	/*get line of file and stores in buffer, validates contents*/
	fgets(buffer, MAX_SIZE, fp);
	send_file_size = strlen(buffer);
	valid = validateFile(buffer, send_file_size);
	if(valid < 0)
	{
		printf("Error: file includes invalid char\n");
		close(sock);
		exit(1);
	}

	/*loop to send file, will continue to send until the send_count equals file size*/
	while(send_count < send_file_size)
	{
		send_bytes = send(sock, buffer, (send_file_size - send_count), 0);
		if(send_bytes < 0)
		{
			send_bytes = 0;
			/*printf("Error sending file \n");
			close(sock);
			exit(1);*/
		}
	send_count += send_bytes;
	}
	fclose(fp);
}

/* 
**	recieves file from server
**	parameters: file and socket
**	returns: none
*/
void getFile(int sock, char *file)
{
	int recv_bytes, total_bytes = 0;
	char buffer[MAX_SIZE];
	char delimiter = '\0';

	/*recieves file in loop until null char is recieved*/
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
}
