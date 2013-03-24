#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

#define READ ( 0 )
#define WRITE ( 1 )

int main(int argc, char const *argv[], char *envp[])
{
	
	int pipe_desc[2];
	int secpipe[2];
	int retval = pipe (pipe_desc);
	retval = pipe (secpipe);
	//long i;
	//Read command line args
	//bool useGrep = true;
	//if (argc < 1)
		//no args
		//useGrep = false;
	//char* env = getenv ();
	//int len = strlen (env);
	int pid = fork ();
	if (pid == 0){
		retval = dup2 (pipe_desc [WRITE], STDOUT_FILENO);
		if (retval == -1){
			printf ("Failed to %s", "blev ingen pipe");
		}
		retval = close (pipe_desc [WRITE]);
		retval = close (pipe_desc [READ]);
		retval = close (secpipe [WRITE]);
		retval = close (secpipe [READ]);
		execlp ("printenv", "printenv", (char *) 0);
		exit (0);
		//child
	}
	pid = fork ();
	if (pid == 0){
		retval = dup2 (pipe_desc [READ], STDIN_FILENO);
		retval = dup2 (secpipe [WRITE], STDOUT_FILENO);
		if (retval == -1)
			printf ("Failed to %s", "dup2");
		retval = close (pipe_desc [READ]);
		retval = close (pipe_desc [WRITE]);
		retval = close (secpipe [WRITE]);
		retval = close (secpipe [READ]);
		execlp ("sort", "sort", (char *) 0);
		exit (0);
	}
	pid = fork ();
	if (pid == 0){
		retval = dup2 (secpipe [READ], STDIN_FILENO);
		retval = close (pipe_desc [READ]);
		retval = close (pipe_desc [WRITE]);
		retval = close (secpipe [READ]);
		retval = close (secpipe [WRITE]);
		printf ("POIPES\n");
		retval = execlp ("/usr/bin/less", "/usr/bin/less", (char *) 0L);
		printf("retval: %d\n", retval);
		printf("%d\n", errno);
		exit (0);
	}
		retval = close (pipe_desc [READ]);
		retval = close (pipe_desc [WRITE]);
		retval = close (secpipe [READ]);
		retval = close (secpipe [WRITE]);
	return retval;
	exit (0);
}