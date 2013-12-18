#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <syslib.h>
#include <ctype.h>
#include <string.h>

int nametonum(char *name)
{
	int i;
	
	for (i=0; name[i] != '\0'; i++)
		/* warning changes actual argv[] toupper */
		name[i] = toupper(name[i]);

	if (strcmp(name, "SIGABRT") == 0) return SIGABRT;  /* abort                         */
	if (strcmp(name, "SIGFPE") == 0)  return SIGFPE ;  /* arithmetic exception          */
	if (strcmp(name, "SIGILL") == 0)  return SIGILL ;  /* illegal instruction           */
	if (strcmp(name, "SIGINT") == 0)  return SIGINT ;  /* attention request from user   */
	if (strcmp(name, "SIGSEGV") == 0) return SIGSEGV;  /* bad memory access             */
	if (strcmp(name, "SIGTERM") == 0) return SIGTERM;  /* termination request           */
	if (strcmp(name, "SIGSTAK") == 0) return SIGSTAK;  /* stack overflow                */
	if (strcmp(name, "SIGALRM") == 0) return SIGALRM;  /* alarm/timeout signal		*/
	if (strcmp(name, "SIGHUP") == 0)  return SIGHUP ;  /* hangup				*/
	if (strcmp(name, "SIGPIPE") == 0) return SIGPIPE;  /* pipe signal			*/
	if (strcmp(name, "SIGQUIT") == 0) return SIGQUIT; 
	if (strcmp(name, "SIGTRAP") == 0) return SIGTRAP;
	if (strcmp(name, "SIGUSR1") == 0) return SIGUSR1;
	if (strcmp(name, "SIGUSR2") == 0) return SIGUSR2;
	if (strcmp(name, "SIGKILL") == 0) return SIGKILL;  /* termination signal		*/

	return -1;
}

void stackcrash(int a)
{
	int fastercrash[10];

	printf("%d ",a);fflush(stdout);
	stackcrash(++a);
}
	
void usage()
{
	printf("usage: sigtest [-n|<signame>|divzero|stack]\n");
	printf("The sigint option raises a pseudo ^C signal\n");
	printf("The stack option causes a real stack overflow\n");
	printf("The divzero option causes a real div by zero\n");
	printf("Other text is taken as the signal name to raise\n\n");
	printf("-n passes a signal number (n) to raise\n\n");
}

int main(int argc, char **argv)
{
	int n,i,j=0;
	

	if (argc <= 1) {
		usage();
		return(1);
	}

	if (*argv[1] == '-')
	{
		if (argc == 2)
			n = atoi(argv[1]+1);
		else
			n = atoi(argv[2]);

		printf("About to raise(%d)\n",n);
		raise(n);
		printf("After raise(%d)\n",n);
	}
	else if ((n = nametonum(argv[1])) != -1)
	{
		printf("Before raise(%d)\n",n);
		raise(n);
		printf("After raise(%d)\n",n);
	}
	else if (strcmp(argv[1],"STACK") == 0)
	{
		printf("About to crash stack: \n");
		stackcrash(0);
		printf("After stack crash!\n");
	}
	else if (strcmp(argv[1],"DIVZERO") == 0)
	{
		printf("About to div by zero!\n");
		i=j/0;
		printf("After div/0!\n");
	}
	else {
		puts("Unknown signal name");
		usage();
	}
}
