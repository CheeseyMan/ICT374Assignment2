#ifndef MENU_C
#define MENU_C

#include "menu.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//Clears teminal screen
void clearMenu()
{
	printf("\033[2J\033[1;1H");
}
//Displays opneing menu
void displayMenu()
{
	clearMenu();
	printf("\n\n\n\n");
	printf("*******************************************");
	printf("\n\n");
	printf("    ICT374 Assignment Two- Simple Shell\n\n");
	printf("          Robert Bessell 33579862\n");
	printf("		     +\n");
	printf("	  Daniel Sharpe  33559778");
	printf("\n\n");
	printf("*******************************************");
	printf("\n\n\n\n");
	sleep(5);
	clearMenu();
}
//Displays prompt
void displayPrompt(char[16] inPrompt)
{	
	printf("%s", inPrompt);
}

#endif
