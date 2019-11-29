#include <common.h>

#include <stdlib.h>
#include <stdio.h>

void quit(char* message)
{
	printf("\n\nSomething went wrong.\n");
	printf("Quit Message: ");
	printf(message);
	printf("\n\nPress return to exit\n");
	getchar();
	exit(-1);
}