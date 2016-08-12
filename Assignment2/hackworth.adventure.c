/*****************************************************************
**File Description: Adventure game that creates random rooms,
**and writes them to file.  Files are then read back in to the game,
**before play.  User transverse the rooms until end room is found.
**Author: Jennifer Hackworth
**Date: 4/21/2016
******************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>

#define MIN_CONNECTIONS 3
#define MAX_CONNECTIONS 6
#define ROOMS_IN_GAME 7

const char *roomNames[] =  {
	"VANQUISHER'S_DEN", 
	"SINNESTER_BURROW", 
	"WEEPING_HOLLOW", 
	"HAPPY_PLACE", 
	"BLOODY_PLAINS",
	"WINDING_CAVERNS",
	"RADIOACTIVE_VILLA",
	"WINTER'S_LAKE",
	"TATTERED_RIDGE",
	"DAISY_PEAK" };

struct Room
{
	const char *name; 								/*name of room*/
	int maxConnections;								/*number of connecting rooms*/
	const char *roomType;									/*start, middle, or end room*/
	struct Room *connectingRooms[MAX_CONNECTIONS];	/*array holds names of connecting rooms*/ 
	int currConnections; 								/*Keeps track of how many connections are currently in connectingRooms array*/
};

void createDirectory();
void randomRoomGenerator(int *randomRoom, int *connections);
void sortArray(struct Room **rooms);
void roomCreator(int *randomRoom, int *connections, struct Room **roomArray);
void connectionCreator(struct Room **roomArray);
void freeRooms(struct Room **rooms);
void writeRoom(struct Room **roomArray);
struct Room *readFile(const char *filename);
void setUpGameRooms(struct Room **rooms);
void gameTime(struct Room **rooms);
const char *userInterface(struct Room *gameRoom);


int main()
{
	int randomRooms[ROOMS_IN_GAME];     	/*array to hold randomly selected room indexes*/
	int randomConnections[ROOMS_IN_GAME];	/*array hold number of connections for each room*/
	struct Room *roomArray[ROOMS_IN_GAME];	/*array to hold rooms before writing*/

	/*seed for random*/
	srand(time(0));

	/*creates a directory for room files*/
	createDirectory();

	/*generated random room names and number of connections*/
	randomRoomGenerator(randomRooms, randomConnections);
	
	/*creates rooms*/	
	roomCreator(randomRooms, randomConnections, roomArray);
	
	/*sorts array by number of connections to ensure a room with 6 can connect to enough rooms*/
	sortArray(roomArray);

	/*Create room connections*/
	connectionCreator(roomArray);	
	
	/*write room information to file*/
	writeRoom(roomArray);

	/*frees structs for rooms created by malloc*/
	freeRooms(roomArray);

	/*reads files in current directory and extracts room info*/
	setUpGameRooms(roomArray);	

	/*game time - start game*/
	gameTime(roomArray);
	
	/*frees dynamically allocated structs contained in array from setUpGameRooms*/
	freeRooms(roomArray);	


	return 0;
}

/* 
**	Creates a directory with the format of <username>.rooms.pid
**	parameters: none
**	returns: none
**	post: directory is created and current working directory is changed
*/
void createDirectory()
{
	int pid = getpid();						/*Process id*/
	char prefix[] = "hackworth.rooms.";		/*first part of directory path*/
	char dirPathName[0];					/*holds directory name

	/*gets Process id and adds it to username.rooms to create directory*/
	sprintf(dirPathName, "%s%d", prefix, pid);
	
	/*Creates new directory*/
	mkdir(dirPathName, 0755);

	/*changes directory path for game files to newly created directory*/
	chdir(dirPathName);
}

/* 
**	Generates a 7 random rooms from from the 10 possible,
**	also creates a random number of connections for said rooms.
**  number generated in random rooms corresponds to name in said index of roomNames.
**	parameters: 2 integer arrays
**	returns: none
**	post: rooms are randomly generated 
*/
void randomRoomGenerator(int *randomRoom, int *connections)
{

	int	roomIndex = 0;
	int i;
	int numSix = 0;		
	do
	{
		int randomNumber = rand() % 10;
		int	randomConnection = 0;

		int selected = 0;
		int i;
		/*checks to see if the room is already randomly selected*/
		for(i = 0; i < roomIndex; i++)
		{
			if(randomRoom[i] == randomNumber)
				selected = 1;
		}
		
		/*if random room selected in unique, places in array to hold rooms for game*/
		if(selected == 0)
		{
			randomRoom[roomIndex] = randomNumber;
			randomConnection = (rand() % (MAX_CONNECTIONS - MIN_CONNECTIONS + 1)) + MIN_CONNECTIONS;
			
			/*if a room has 6 connections, it will connect to every other room, since the min connections is 3,
			 *Only 3 room can have 6 connections.  Because of the bi directional nature of the connections,
			 *if a room has 3 connections it will not be able to connect back to 
			 *4 rooms with 6 connections*/
			if(randomConnection == 6)
			{
					if(numSix == 3)
					{
						while(randomConnection == 6)
							randomConnection = (rand() % (MAX_CONNECTIONS - MIN_CONNECTIONS + 1)) + MIN_CONNECTIONS;							
					}	
					else 
						numSix++;
			}
	
			connections[roomIndex] = randomConnection;	
			roomIndex++;		
		}

	}while(roomIndex < ROOMS_IN_GAME);

}

/* 
**	Generates a struct with the room infomation taken from 2 integer arrays.
**	places structs in an array
**	parameters: 2 integer arrays, struct Room array
**	returns: none
**	post: room structs with room infomation are stored
*/
void roomCreator(int *randomRoom, int *connections, struct Room **roomArray)
{
	
	int i, j;
	for(i = 0; i < ROOMS_IN_GAME; i++)
	{
		struct Room *gameRoom = malloc(sizeof(struct Room));    //creates a struct to hold room info
		gameRoom->name = roomNames[randomRoom[i]];              //grabs room name from roomName array
		gameRoom->maxConnections = connections[i];              //sets max connections room can have
		gameRoom->currConnections = 0;                          //sets current connections to 0
		

		/*gives rooms a type label by position in array
         *If room in first in array it is start room, last end,
         *all others, mid
         */
		if(i == 0)
			gameRoom->roomType = "START_ROOM";
		else if(i == (ROOMS_IN_GAME - 1))
			gameRoom->roomType = "END_ROOM";
		else
			gameRoom->roomType = "MID_ROOM";

		roomArray[i] = gameRoom;
	}
}	

/* 
**	creates connections between rooms
**	parameters: struct Room array
**	returns: none
**	post: connections between rooms are made
*/
void connectionCreator(struct Room **roomArray)
{
	int i, j;

	/*creates connections between rooms*/
	for(i = 0; i < ROOMS_IN_GAME; i++)
	{
		/*if room has 6 connections, creates a connection between each of the other rooms*/
		if(roomArray[i]->maxConnections == 6)
		{
			for(j = i + 1; j < ROOMS_IN_GAME; j++)
			{
				roomArray[i]->connectingRooms[roomArray[i]->currConnections] = roomArray[j];       
				roomArray[j]->connectingRooms[roomArray[j]->currConnections] = roomArray[i];	
				roomArray[i]->currConnections++;
				roomArray[j]->currConnections++;
			}		
		}
		
		/*if less than 6 connections, finds rooms that still have room for connections and connects!*/
		else
		{
			if(roomArray[i]->maxConnections > roomArray[i]->currConnections)
			{
				for(j = i + 1; j < ROOMS_IN_GAME; j++)
				{
					if(roomArray[j]->maxConnections > roomArray[j]->currConnections)
					{				
						roomArray[i]->connectingRooms[roomArray[i]->currConnections] = roomArray[j];       
						roomArray[j]->connectingRooms[roomArray[j]->currConnections] = roomArray[i];	
						roomArray[i]->currConnections++;
						roomArray[j]->currConnections++;
					}
				}
				/*adjusts number of connections if there  are not enough rooms to connect to*/
				if(roomArray[i]->maxConnections < roomArray[i]->currConnections)			
					roomArray[i]->maxConnections = roomArray[i]->currConnections;	
			}				
		}
	}
		
}	

/* 
**	sorts array in order of max connections.
**  This is to help ensure that rooms with 
**	6 connections connect first to other rooms.
**	parameters: struct Room array
**	returns: none
**	post: array is sorted
*/
void sortArray(struct Room **rooms)
{
	int i, j, maxIndex, maxValue;
	struct Room *temp;

	/*selection sort algorithm used to sort array*/

	for(i = 0; i < ROOMS_IN_GAME - 1; i++)
	{
		maxIndex = i;								
		maxValue = rooms[i]->maxConnections;
		temp = rooms[i];

		for(j = i + 1; j < ROOMS_IN_GAME; j++)
		{
			if(rooms[j]->maxConnections > maxValue)
			{
				maxValue = rooms[j]->maxConnections;
				maxIndex = j;
			}
		}

		temp = rooms[maxIndex];
		rooms[maxIndex] = rooms[i];
		rooms[i] = temp;
	}
}	

/* 
**	Writes room info to files stored in current working directory
**	parameters: struct Room array
**	returns: none
**	post: rooms are written to files
*/
void writeRoom(struct Room **roomArray)
{

	int i, j;
	const char *filename;
	
	/*Cycles through array and writes each room info to file*/
	for(i = 0; i < ROOMS_IN_GAME; i++)
	{	
			filename = roomArray[i]->name;				//sets filename to room name

		FILE *outputFile = fopen(filename, "w");

		if(outputFile == NULL)							//checks to make sure file was opened
		{
			printf("Error opening room file");
			exit(1);
		}

		/*writes room data*/
		fprintf(outputFile, "ROOM NAME: %s\n", roomArray[i]->name);		


	for(j = 0; j < roomArray[i]->currConnections; j++)
		fprintf(outputFile, "CONNECTION %d: %s\n", i + 1,roomArray[i]->connectingRooms[j]->name);
	
	fprintf(outputFile, "ROOM TYPE: %s\n", roomArray[i]->roomType);
	
	fclose(outputFile);	
	}
}

/* 
**	frees dyamically allocated structs
**	parameters: struct Room array
**	returns: none
**	post: memory is freed
*/
void freeRooms(struct Room **rooms)
{
	int i;
		for(i = 0; i < sizeof(rooms) - 1; i++)
			free(rooms[i]);
}

/* 
**	reads room info in from file
**	parameters: file name
**	returns: struct Room
**	post: struct is created and populated with room info
*/
struct Room *readFile(const char *filename)
{
	struct Room *gameRoom = malloc(sizeof(struct Room));
	int i, counter = 0;	
	char line[50];
	char input[30];


	FILE *roomFile = fopen(filename, "r");
	
	/*reads line in from file*/
	while(fgets(line, sizeof(line), roomFile) != NULL)
	{
		/*tests first 9 chars from file agianst to tell if the line gives
		 *room name, connection, or room type info.
		 *Once line type is determined, sscanf extracts string from line.
		 *A char array is dyamically allocated to hold a copy of string, that is used to set 
         *variable in the struct.
         */
		if(strncmp(line, "ROOM NAME", 9) == 0)
		{
			sscanf(line, "ROOM NAME: %s\n", input);
			char *temp = malloc(strlen(input));				
			strcpy(temp, input);
			gameRoom->name = temp;
			

		}
		else if(strncmp(line, "CONNECTION", 9) == 0)
		{

			gameRoom->connectingRooms[counter] = malloc(sizeof(struct Room));
		
			sscanf(line, "CONNECTION %d: %s\n", &i, input);
			char *temp = malloc(strlen(input));	
			strcpy(temp, input);

			gameRoom->connectingRooms[counter]->name = temp;

			counter++;
		}

		else
		{
			sscanf(line, "ROOM TYPE: %s\n", input);
			char *temp = malloc(strlen(input));	
			strcpy(temp, input);
			gameRoom->roomType = temp;
		}
	}

	fclose(roomFile);

	gameRoom->currConnections = counter;               //sets the number of outgoing connections room has

	return gameRoom;							
}

/* 
**	Reads names from working director
**	parameters: struct Room array
**	returns: none
**	post: directory is read and files created
*/
void setUpGameRooms(struct Room **rooms)
{
	DIR *d;
	struct dirent *dir;
	d = opendir(".");
	int index = 0;  
	const char *filename;

	if (d)
  	{
		/*reads files in directory until null is incountered*/
    	while ((dir = readdir(d)) != NULL)
    	{
			if(dir->d_name[0] != '.')         				 //skips files that start with a .
			{
	    		filename = dir->d_name;       				//sets filename to the filename currently being read in directory
				rooms[index] = readFile(filename);        //calls read file and puts returned struct in array
				index++;                                  //updates index counter
			}

		}

    	closedir(d);
	}
}

/* 
**	Plays though the game
**	parameters: struct Room array
**	returns: none
**	post: game is played
*/
void gameTime(struct Room **rooms)
{
	struct Room *currRoom;			   //holds current room info (room player is currently in
	int roomPath[50];                  //holds an integer that corresponds to an index in rooms
	int i, counter, steps = 0;
	const char *userInput;

	/*PLAYS GAME!!!*/
	do
	{
		/*if steps is 0, searches rooms for a room with time START_ROOM.
         *sets this as the current room.
		 */
		if(steps == 0)
		{
			for(i = 0; i < ROOMS_IN_GAME; i++)
			{
				if(strcmp(rooms[i]->roomType, "START_ROOM") == 0)
					currRoom = rooms[i];
			}
		}		

		else
		{ 
			/*searches array for room indicated by user, sets current room*/
			for(i = 0; i < ROOMS_IN_GAME; i++)
			{
				if(strcmp(rooms[i]->name, userInput) == 0)
				{	
					currRoom = rooms[i];
					roomPath[steps - 1] = i;
				}	
			}
		}

		/*game can only track 50 steps*/
		if(steps == 50)
		{
			printf("YOU REALLY COULDN'T SOLVE THIS SIMPLE MAZE IN 50 TURNS....HOW VERY SAD.\n");
			return;
		}

		/*if player has found end room, display game info*/
		if(strcmp(currRoom->roomType, "END_ROOM") == 0)
		{

			printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\n");
			printf("YOU TOOK %d STEPS.  YOUR PATH TO VICTORY WAS:\n", steps);
		
			for(i = 0; i < steps; i++)	
			{
				counter = roomPath[i];
				printf("%s\n", rooms[counter]->name);
			}
		}

		/*calls function to give user room options*/
		else
		{
			userInput = userInterface(currRoom);
			steps++;
		}	
	
	}while(strcmp(currRoom->roomType, "END_ROOM") != 0);

}

/* 
**	Displays room info to user, and excepts user input
**	parameters: struct Room
**	returns: const char*
**	post: memory is freed
*/
const char *userInterface(struct Room *gameRoom)
{
	char userInput[30];
	int validEntry = 0;

	do
	{
		/*prints room info to console*/
		printf("\n");
		printf("CURRENT LOCATION: %s\n", gameRoom->name);
	
		printf("POSSIBLE CONNECTIONS: ");

		int i;
		for(i = 0; i < gameRoom->currConnections; i++)
		{
			if((i + 1) < gameRoom->currConnections)
				printf("%s, ", gameRoom->connectingRooms[i]->name);

			else	
				printf("%s.\n", gameRoom->connectingRooms[i]->name);
		}
	
		printf("WHERE TO?> ");

		fgets(userInput, 30, stdin);	

		/*removes newline char from user input*/	
		int last = strlen(userInput)-1;
   		if(userInput[last]=='\n') userInput[last] = '\0';

		/*makes sure room entered by user exists in game, if not error message is displayed*/
		for(i = 0; i < gameRoom->currConnections; i++)
		{
			if(strcmp(userInput, gameRoom->connectingRooms[i]->name) == 0)
			{	
				validEntry = 1;
				return gameRoom->connectingRooms[i]->name;				
			}
		}
		
		if(validEntry == 0)
			printf("HUH? I DON'T UNDERSTAND HAT ROOM.  TRY AGAIN \n");

	printf("\n");
	}while(validEntry == 0);

}		
