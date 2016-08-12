/*****************************************************************
**File Description: Shell program for Linux
**Can handle basic file/directory manipulation
**Author: Jennifer Hackworth
**Date: 5/15/2016
******************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#define BUFFER_SIZE 2048
#define MAX_ARGUMENTS 512
#define BG_MAX 100

void loop();
int parseLine(char *line, char **arguments);
int  execute(char **arguments, pid_t *chList, int *lastStatus, int marker);
int changeDirectory(char **arguments);
int exitSh(int *chList);
int checkSignal(int *lastStatus);
int  launch(char **arguments, pid_t *chList, int *lastStatus, int marker);
void pushBG(pid_t *chList, pid_t pid);
void bgCheck(pid_t *chList);
int redirect(char **arguments, int putType, int background);

int main()
{
	/*signal handling set up.
	 *allows interrupt signals to 
	 *stop process in the foreground
	 *without killing the shell or background processes
	 */
	struct sigaction act;
	act.sa_flags = 0;
	act.sa_handler = SIG_IGN;
	sigaction(SIGINT, &act, NULL);


	loop();

	return 0;
}

/* 
**	allows user to loop though comands without exiting shell
**	parameters: none
**	returns: none
**	post: none
*/
void loop()
{
	char *line;											//holds user input
	char **args;										//vector to store commands entered
	int i, status, marker;								//variables to handle returned values
	pid_t *chList = malloc(sizeof(pid_t) * BG_MAX);		//stores background processes
	int *lastStatus = malloc(sizeof(int));				//stores status of last foreground process

	for(i = 0; i < BG_MAX; i++)
		chList[i] = 0;

	/*loop controls shell*/
	do
	{
		bgCheck(chList);								//checks to see if any background processes have ended
		printf(": ");
		
		args = malloc(sizeof(char*) * MAX_ARGUMENTS);
		line = malloc(BUFFER_SIZE);
		
		fflush(stdout);		

		fgets(line, BUFFER_SIZE, stdin);				//gets user input

		marker = parseLine(line, args);					//"breaks" apart line into commands and arguments

		status = execute(args, chList, lastStatus, marker);  //executes command and returns status.  Shell exits on status = 0.

		free(line);
		free(args);

	}while(status);
}

/* 
**	turns user input into a vector of commands and arguments
**	parameters: char line and array
**	returns: number of arguments in array
**	post: arguments contained in array
*/
int parseLine(char *buffer, char **arguments)
{
	int counter = 0;

	/*loops through line to get arguments*/
	arguments[counter] = strtok(buffer, " \n");

	while(arguments[counter] != NULL)
	{	
		counter++;
		arguments[counter] = strtok(NULL, " \n");
	}
	
	counter--;
	return counter;
}

/* 
**	Determines which function is needed to execute the command
**	parameters: argument array, background process array, last foreground status, and number of arguments
**	returns: exit status of executed command
**	post: command is executed and status returned
*/
int  execute(char **arguments, pid_t *chList, int *lastStatus, int marker)
{
	/*if the line is empty or starts with a # (comment symbol) returns 1 to loop agian*/
	if(arguments[0] == NULL || strcmp(arguments[0], "#") == 0)
		return 1;

	/*if command is cd calls, change directory function*/
	else if(strcmp(arguments[0], "cd") == 0)
		return changeDirectory(arguments);
	
	/*if command is status, calls status function*/
	else if(strcmp(arguments[0], "status") == 0)
		return checkStatus(lastStatus);

	/*if command is exit, calls function to kill children then returns 0*/
	else if(strcmp(arguments[0], "exit") == 0)
		return exitSh(chList);

	/*any other command is not built in, so generic launch is called*/
	else
		return launch(arguments, chList, lastStatus, marker);
}

/* 
**	changes users working directory
**	parameters: argument array
**	returns: exit status
**	post: users working directory is changed
*/
int changeDirectory(char **arguments)
{
	/*if there are no other arguments besides cd, changes directory to home directory*/
	if(arguments[1] == NULL)
		chdir(getenv("HOME"));

	/*changes directory to the argument entered after cd*/
	else
		chdir(arguments[1]);

	return 1;
}

/* 
**	kills children and then returns status to exit shell
**	parameters: background child list
**	returns: exit status
**	post: children are killed, exit status 0 returned.
*/
int exitSh(pid_t *chList)
{
	/*Loops though chlist and sends kill signal to any processes*/
	int i;
		for(i = 0; i < BG_MAX; i++)
		{
			if(chList[i] > 0)
				kill(chList[i], SIGKILL);
		}

	free(chList);

	return 0;
}

/* 
**	Displays status for last foreground process to the console
**	parameters: lastStatus integer
**	returns: exit status
**	post: none
*/
int checkStatus(int *lastStatus)
{
	int status;
	
	status = *lastStatus;

	/*if status ended normally returns exit value*/
	if(WIFEXITED(status))
		printf("Exited value: %d.\n", WEXITSTATUS(status));

	/*if status was killed by signal, returns signal number*/	
	else if(WIFSIGNALED(status))	
		printf("terminated by signal %d.\n", WTERMSIG(status));
	return 1;
}

/* 
**	controls flow for non built in commands, forking, opening files, and all that good stuff
**	parameters: argument array, background process array, last foreground status, and number of arguments
**	returns: exit status
**	post: command executed
*/
int launch(char **arguments, pid_t *chList, int *lastStatus, int marker)
{
	pid_t pid, wpid;   										//holds pid of fork
	int status;												//holds status of child

	pid = fork();											
	int fd, putType = -1, background = 0;					//hold file descriptor, type of file to open (none, read, write) and background process bool

	/*checks to see if process should be ran in the background.
	 *If so, changes background bool to true and nulls last argument in arg array
	 *pushes pid on the chList
	 */
	if(strcmp(arguments[marker], "&") == 0)
	{
		background = 1;
		arguments[marker] = NULL;
		marker--;
		pushBG(chList, pid);
	}
	
	/*checks to see if any files need to be opened, if so calls function to do so*/
	if(arguments[1] != NULL || background == 1)
	{
		/*putType equal to -1: no output/input file declared
		 *putType equal to 1:output file specified
		 *putType equal to 0: input file specified
 	     */	

		if(strcmp(arguments[1], ">") == 0)
			putType = 1;
 	
		if(strcmp(arguments[1], "<") == 0)
			putType = 0;
					
		fd = redirect(arguments, putType, background);	
	}
	

	if(pid == 0)
	{
		/*sets signal handling, background process will ignore. otherwise default behavoir*/
		if(background == 1)
			signal(SIGINT, SIG_IGN);


		else
			signal(SIGINT, SIG_DFL);

		/*if there is a file open dups file before calling execvp*/
		if(putType == 1)
			dup2(fd, 1);

		else if(putType == 0)
			dup2(fd, 0);

		if(execvp(arguments[0], arguments) == -1)
		{
			printf("Command of file not recognized\n");
			exit(1);
		}
	}

	else if(pid < 0)
		printf("Error Forking\n");
	
	/*parent process*/
	else
	{
		/*if child is not background - waits for process to finish*/
		if(background == 0)
		{
			do
			{
				wpid = waitpid(pid, &status, WUNTRACED);
			}while(!WIFEXITED(status) && !WIFSIGNALED(status));

			*lastStatus = status;		
	
			/*if child terminated by signal - prints message*/
			if(WIFSIGNALED(status))			
			{
				printf("Terminated by Signal %d\n", WTERMSIG(status));
				fflush(stdout);	
			}
		}
		
		/*if background process - prints process id*/
		else
		{
			printf("Background Pid is: %d\n", pid);
			fflush(stdout);
		}

	}

	return 1;
}	

/* 
**	checks to see if any background processes have finished, if so displays info to console
**	parameters: background process array
**	returns: none
**	post: none
*/
void bgCheck(pid_t *chList)
{
	int i, status;
	pid_t pid = waitpid(-1, &status, WNOHANG);							//checks to see if any processes have finished

	/*if any processes have finished, loops though array to find pid.
	 *if found, will print pid number and exit status or termiation signal number.
	 */
	if(pid > 0)
	{
		for(i = 0; i < BG_MAX; i++)
		{
			if(chList[i] == pid)
			{
				printf("Background pid #%d is done: ", pid);
	
				if(WIFEXITED(status))
					printf("Exited value: %d.\n", WEXITSTATUS(status));
		
				else if(WIFSIGNALED(status))	
					printf("terminated by signal %d.\n", WTERMSIG(status));
		
				chList[i] = 0;												//removes pid if found from array

				return;
			}
		}
	}
}

/* 
**	adds background pid to array
**	parameters: background process array, pid
**	returns: none
**	post: pid added to array
*/
void pushBG(pid_t *chList, pid_t pid)
{
	int i;
	
	for(i = 0; i < BG_MAX; i++)
	{
		if(chList[i] == 0)
		{
			chList[i] = pid;
			return;
		}


	}

}		

/* 
**	opens files for redirection or directs background operations to /dev/null
**	parameters: argument array, putType, background
**	returns: file descriptor
**	post: file is opened for read or write
*/
int redirect(char **arguments, int putType, int background)
{
	int fd;																									

	/*if 2 argument is a ">" opens file for writing, prints error message if error opening file*/
	if(putType == 1)
	{
		fd = open(arguments[2], O_WRONLY | O_CREAT | O_TRUNC, 0664);
	
		if(fd == -1)
		{
			printf("Error opening file\n");
			exit(1);
		}

		/*sets 2nd and 3rd argument to NULL since they do not get past into execvp*/
		arguments[1] = NULL;
		arguments[2] = NULL;
	}


	/*if 2 argument is a "<" opens file for reading, prints error message if error opening file*/
	else if(putType = 0)
	{	
		fd = open(arguments[2], O_RDONLY);

		if(fd == -1)
		{
			printf("Error opening file\n");
			exit(1);
		}

		/*sets 2nd and 3rd argument to NULL since they do not get past into execvp*/
		arguments[1] = NULL;
		arguments[2] = NULL;
	}
	
	/*if process is a background process, opens file for read and write to /dev/null, if file not already open*/
	if(background == 1) 
	{
		if(putType != 1)
			fd = open("/dev/null", O_RDONLY);
	
		if(putType != 0)
			fd = open("/dev/null", O_WRONLY);
	}

	return fd;			
}
		
	
