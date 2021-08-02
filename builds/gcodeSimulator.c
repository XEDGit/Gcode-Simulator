#include "gcode.h"

//options
int fbfMode = 0;

//main menu
int menuIndex = 0;
int indexBkp = 0;
int subMenu = 0;

//file finder
DIR *directory;
struct dirent *dirContent;
int fileNumber;
char fileList[100][50];

//simulator
char brokenCommand[10][40];
char simulatedLine[59] = "|                                                       |\n";
typedef struct
{
	int x, y, z, speed, feedRate;
	float pFeedRate;
} Agent;
Agent agente = {0};
int moved;
typedef struct
{
	int x, y;
} History;
History tail[20000];
int lastHRounds;
unsigned int rounds = 0;
struct stat checkFolder = {0};
int speed = 1;

//utility
void clr()
{
	system("cls");
}

//clear filament option
void clearTail()
{
	int count = 0;
	while (count <= lastHRounds)
	{
		tail[count].x = '\0';
		tail[count].y = '\0';
		count++;
	}
}

//print res[][] array containing rendered frame
void printResult(char res[29][59])
{

	clr();
	int tttt = 2;

	printf("_________________________________________________________\n\n");
	printf("\t\t\033[0;34mGcode Simulator\033[0;37m\n\n\n");
	printf("\t\t  \033[0;36mSimulator\033[0;37m\n\n");
	printf(" _______________________________________________________\n");

	while (tttt < 28)
	{
		printf("%s", res[tttt]);
		tttt++;
	}

	printf(" _______________________________________________________\n");

	printf("\n\n\t\033[0;33mSimulation speed: x%d", speed);
	printf("\n\n\tPrinter speed: %d", agente.speed);
	printf("\n\tFeed Rate: %d\033[0;37m", agente.feedRate);
	printf("\n\t\033[0;32mX: %d\tY: %d\tZ: %d\033[0;37m", agente.x, agente.y, agente.z);
	if (fbfMode == 1)
	{
		printf("\n\n\tPress \033[0;33many key\033[0;37m to see next frame, \033[0;31mX\033[0;37m to exit");
	}
	else
	{
		printf("\n\n\t\033[0;33mOPTIONS\n\tV: Pause\tC: Clear filament trace\n\tX: Toggle frame-by-frame mode\033[0;37m\n\t\033[0;35mN: Decrease Sim. speed\tM: Increase Sim. speed\033[0;37m\n\t\033[0;31mZ: Exit\033[0;37m");
	}
}

//interpretate string command and render frame and trace based on it
int printSimulation(char line[], int *index)
{

	//break down line in single command into brokencommand[]
	unsigned int counter = 0;
	int commandCounter = 0;
	int commandSubCounter = 0;
	int skipDecimals = 0;
	char result[29][59];

	if (line[0] == ';')
	{
		return (0);
	}

	while (strlen(line) > counter)
	{
		if (line[counter] == '.')
		{
			skipDecimals = 1;
		}

		if (line[counter] == ' ')
		{
			commandCounter++;
			commandSubCounter = 0;
			skipDecimals = 0;
		}
		else if (skipDecimals == 0)
		{
			brokenCommand[commandCounter][commandSubCounter] = line[counter];
			commandSubCounter++;
		}
		else
		{
			brokenCommand[commandCounter][commandSubCounter] = '\0';
		}
		counter++;
	}
	rounds++;

	//read and execute single commands
	int brokenCounter = 0;
	while (brokenCounter <= commandCounter)
	{
		if (brokenCommand[brokenCounter][0] == ';')
		{
			break;
		}

		char digits[6];
		strcpy(digits, brokenCommand[brokenCounter]);
		memmove(&digits[0], &digits[1], strlen(digits));
		int number;
		number = atoi(digits);

		switch (brokenCommand[brokenCounter][0])
		{
		case 'F':
			agente.feedRate = number;
			break;
		case 'S':
			agente.speed = number;
			break;
		case 'X':
			if ((int)agente.x / 4 != (int)number / 4)
			{
				moved = 1;
			}
			agente.x = number;
			break;
		case 'Y':
			if ((int)agente.y / 8 != (int)number / 8)
			{
				moved = 1;
			}
			agente.y = number;
			break;
		case 'Z':
			agente.z = number;
			break;
		default:
			break;
		}
		brokenCounter++;
	}

	//print result
	if (moved == 0)
	{
		return (1);
	}

	int lineCounter = 0;
	while (lineCounter < 28)
	{
		char modifiedLine[59];
		strcpy(modifiedLine, simulatedLine);

		unsigned int tailCounter = 0;
		while (tailCounter < (sizeof(tail) / sizeof(tail[0])) - 2 && tailCounter != rounds)
		{
			if (tail[tailCounter].y / 8 == lineCounter)
			{
				modifiedLine[(tail[tailCounter].x / 4) + 1] = '-';
			}
			tailCounter++;
		}

		if (agente.y / 8 != lineCounter)
		{
			strcpy(result[lineCounter], modifiedLine);
		}
		else
		{
			modifiedLine[(agente.x / 4) + 1] = 'O';
			strcpy(result[lineCounter], modifiedLine);
		}
		lineCounter++;
	}

	//frame by frame mode
	if (fbfMode == 1)
	{
		char x = getch();
		if (x == 'x')
		{
			fbfMode = 0;
		}
	}

	//speed option
	if (*index >= speed)
	{
		*index = 1;
	}
	else
	{
		*index = *index + 1;
		return (1);
	}
	if (speed < 1)
	{
		speed = 1;
	}

	//printing funtion
	printResult(result);

	return (1);
}

void historySimulation()
{

	int contTemp = 0;
	int roundTemp = 0;
	while (tail[roundTemp].x != '\0' && tail[roundTemp].x != '\0')
	{
		if (agente.x / 4 == tail[contTemp].x / 4 && agente.y / 8 == tail[contTemp].y / 8)
		{
			roundTemp++;
			continue;
		}

		roundTemp++;
		contTemp++;
	}
	lastHRounds = roundTemp;
	History temp = {0, 0};
	temp.x = agente.x;
	temp.y = agente.y;
	tail[contTemp] = temp;
}

void escape()
{
	menuIndex = 0;
	subMenu = 0;
	speed = 1;
}

void simulate()
{
	char fullPath[50] = "./Gcode Simulator Files/";
	strcat(fullPath, fileList[fileNumber]);
	FILE *file = fopen(fullPath, "r");
	char line[256];
	int index = 1;
	if (file != NULL)
	{
		while (fgets(line, sizeof(line), file))
		{
			if (printSimulation(line, &index) == 1)
			{
				historySimulation();
			}
			if (kbhit() != 0 && fbfMode == 0)
			{
				char key = getch();
				if (key == 'z')
				{
					printf("\n\tSimulation Terminated, press Enter to get back to Main Menu...\n");
					escape();
					break;
				}
				else if (key == 'x')
				{
					if (fbfMode == 0)
					{
						speed = 1;
						fbfMode++;
						clr();
						printf("\n\n\t\tPress \033[0;33many key\033[0;37m to see next frame, \033[0;31mX\033[0;37m to exit");
					}
					else
					{
						fbfMode--;
					}
				}
				else if (key == 'v')
				{
					printf("\n\n\tPress \033[0;33mEnter\033[0;37m to restart...");
					getchar();
				}
				else if (key == 'c')
				{
					clearTail();
				}
				else if (key == 'm')
				{
					speed++;
				}
				else if (key == 'n')
				{
					speed--;
				}
			}
		}
		clearTail();
		agente.x = 0;
		agente.y = 0;
		agente.z = 0;
		rounds = 0;
		fclose(file);
		getchar();
	}
}

void printMenu()
{
	clr();
	switch (menuIndex)
	{

	//main menu
	case 0:
		printf("_________________________________________________________\n\n");
		printf("\tWelcome to \033[0;34mGcode Simulator\033[0;37m written in \033[0;32mC\033[0;37m \n\t\tby \033[0;36mXEDGit\033[0;37m\n\n");
		printf("\tChoose an option: (1: \033[0;32mSimulate\033[0;37m |2: \033[0;31mExit\033[0;37m)\n");
		scanf("%d", &menuIndex);

		while (menuIndex > 2 || menuIndex < 0)
		{
			menuIndex = 0;
			printf("\033[0;31mChoose a valid option:\033[0;37m \n");
			scanf("%d", &menuIndex);
		}
		getchar();
		break;

	//simulator
	case 1:
		switch (subMenu)
		{
			//choose file
		case 0:
			printf("_________________________________________________________\n\n");
			printf("\t\t\033[0;34mGcode Simulator\033[0;37m\n\n\n");
			printf("\t\t  \033[0;36mSimulator\033[0;37m\n\n");

			directory = opendir("./Gcode Simulator Files");
			int label = 0;

			while ((dirContent = readdir(directory)) != NULL)
			{
				printf("\t%d\t", label);
				strcpy(fileList[label], dirContent->d_name);
				printf("%s\n", dirContent->d_name);
				label += 1;
			}
			printf("\t%d\tBack\n\n", label);

			closedir(directory);
			printf("\tChoose file number:\n");
			scanf("\t%d", &fileNumber);
			while (fileNumber > label || fileNumber < 2)
			{
				printf("\t\033[0;31mError. Insert valid file number:\033[0;37m\n");
				scanf("\t%d", &fileNumber);
				getchar();
			}

			if (fileNumber == label)
			{
				escape();
				label = 0;
				return;
			}

			subMenu = 1;
			label = 0;
			break;

		//read and simulate selected file
		case 1:
		
			simulate();
			getchar();
			escape();

			break;
		}

		break;

	case 2:

		exit(0);
	}
	return;
}

int main()
{
	//check souce folder existance and create if not
	agente.speed = 100;
	agente.x = 0;
	agente.y = 0;

	if (stat("./Gcode Simulator Files", &checkFolder) == -1)
	{
		mkdir("./Gcode Simulator Files");
	}

	//error handling
	while (menuIndex != 5)
	{
		if (menuIndex > 3 || menuIndex < 0)
		{
			escape();
		}

		printMenu();
	}
	return (0);
}