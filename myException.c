#include "myException.h"
#include "MainHeader.h"

void childHandler()
{
	int status;
	int spid;
	spid = wait(&status);
}

void printError(char *message)
{
	perror(message);
	exit(1);
}
