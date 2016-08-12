/*****************************************************************
**File Description: Randomly generates a key to use for encryption
**Takes an integer for the number of letters to generate as a parameter
**Author: Jennifer Hackworth
**Date: 5/25/2016
******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void otpGen(char *argv[]);

int main(int argc, char *argv[])
{
	/*seed for random numbers*/
	srand(time(0));

	/*checks to see if the right number of parameters are entered*/
	if(argc != 2)
	{
		printf("Error: Incorrect number of arguments\n");
		exit(1);
	}

	else
		otpGen(argv);

	return 0;
}

/* 
**	Generates specified number of arguments
**	parameters: parameter array
**	returns: none
*/
void otpGen(char *argv[])
{
	int i;
	char keyChar;
	int keyNum;

	/*finds length of key*/
	int keyLength = atoi(argv[1]);

	/*loops through to generate the corret number of letters*/
	for(i = 0; i < keyLength; i++)
	{
		keyNum = (rand() % 27);

		/*assigns a letter based of Alphabet number 0 = A 26 = " "*/
		if(keyNum == 26)
			keyChar = ' ';
		else
			keyChar = keyNum + 'A';

		printf("%c", keyChar); 
	}

	printf("\n");
}
