#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>

#define READ ( 0 )
#define WRITE ( 1 )

int main(int argc, char *argv[], char *envp[])
{
	char *printenv[] = {"printenv", NULL};
	char *sort[] = {"sort", NULL};
	char *less[] = {"less", NULL};
	char *params[100];
	int i;
	params[0] = "grep";
	params[1] = "-E";
	char pattern[80];
	strcpy (pattern,"");
	for (i=1;i<argc-1;i++){
		//strcat (pattern, "(");
		strcat (pattern, argv[i]);
		strcat (pattern, "|");
	}
	strcat (pattern, argv[argc-1]);
	params[2] = pattern;
	params[3] = NULL;
	int first_pipe[2];
	int second_pipe[2];
	int third_pipe[2];
	int retval = pipe (first_pipe);
	retval = pipe (second_pipe);
	retval = pipe (third_pipe);

	//long i;
	//Read command line args
	bool useGrep = true;
	if (argc < 2){
		useGrep = false;
		printf ("NOARGS\n");
	} else{
		printf ("ARGS\n");
	}
	int pid = fork ();
	if (pid == 0){
		if (useGrep)
			retval = dup2 (first_pipe [WRITE], STDOUT_FILENO);
		else
			retval = dup2 (second_pipe[WRITE], STDOUT_FILENO);
		if (retval == -1){
			printf ("Failed to %s", "blev ingen pipe");
		}
		retval = close (first_pipe [WRITE]);
		retval = close (first_pipe [READ]);
		retval = close (second_pipe [WRITE]);
		retval = close (second_pipe [READ]);
		retval = close (third_pipe [WRITE]);
		retval = close (third_pipe [READ]);
		execvp (*printenv, printenv);
		exit (0);
		//child
	}
	if (useGrep){
		pid = fork ();
		if (pid == 0){
			//printf ("%s, %s, %s", params[0], params[1], params[2]);
			retval = dup2 (first_pipe [READ], STDIN_FILENO);
			retval = dup2 (second_pipe [WRITE], STDOUT_FILENO);
			if (retval == -1)
				printf ("Failed to %s", "dup2");
			retval = close (first_pipe [READ]);
			retval = close (first_pipe [WRITE]);
			retval = close (second_pipe [WRITE]);
			retval = close (second_pipe [READ]);
			retval = close (third_pipe [WRITE]);
			retval = close (third_pipe [READ]);
			execvp (*params, params);
			exit (0);
		}
	}
	pid = fork ();
	if (pid == 0){
		retval = dup2 (second_pipe [READ], STDIN_FILENO);
		retval = dup2 (third_pipe [WRITE], STDOUT_FILENO);
		if (retval == -1)
			printf ("Failed to %s", "dup2");
		retval = close (first_pipe [READ]);
		retval = close (first_pipe [WRITE]);
		retval = close (second_pipe [WRITE]);
		retval = close (second_pipe [READ]);
		retval = close (third_pipe [WRITE]);
		retval = close (third_pipe [READ]);
		execvp (*sort, sort);
		exit (0);
	}
	pid = fork ();
	if (pid == 0){
		retval = dup2 (third_pipe [READ], STDIN_FILENO);
		retval = close (first_pipe [READ]);
		retval = close (first_pipe [WRITE]);
		retval = close (second_pipe [WRITE]);
		retval = close (second_pipe [READ]);
		retval = close (third_pipe [READ]);
		retval = close (third_pipe [WRITE]);
		retval = execvp (*less, less);
		printf("retval: %d\n", retval);
		printf("%d\n", errno);
		exit (0);
	}
	retval = close (first_pipe [READ]);
	retval = close (first_pipe [WRITE]);
	retval = close (second_pipe [WRITE]);
	retval = close (second_pipe [READ]);
	retval = close (third_pipe [READ]);
	retval = close (third_pipe [WRITE]);
	int status;
	for (i = 0; i < 4; i++)
		wait(&status);
	exit (0);
}