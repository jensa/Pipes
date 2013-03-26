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

int first_pipe[2];
int second_pipe[2];
int third_pipe[2];

void err (char * msg){
	fprintf (stderr, "%s",msg);
}
/* close_pipes
*
* close_pipes closes the ends of all pipes.
*
*/
void close_pipes (){
	int retval = close (first_pipe [WRITE]);
	if (retval == -1)
		err ("failed to close pipe");
	retval = close (first_pipe [READ]);
	if (retval == -1)
		err ("failed to close pipe");
	retval = close (second_pipe [WRITE]);
	if (retval == -1)
		err ("failed to close pipe");
	retval = close (second_pipe [READ]);
	if (retval == -1)
		err ("failed to close pipe");
	retval = close (third_pipe [WRITE]);
	if (retval == -1)
		err ("failed to close pipe");
	retval = close (third_pipe [READ]);
	if (retval == -1)
		err ("failed to close pipe");
}

int main(int argc, char *argv[], char *envp[])
{
	bool useGrep = true; /* used to indicate whether grep should be called*/
	if (argc < 2) /*find out whether any arguments were used (i.e. grep should be called) */
	useGrep = false;
	int i; /* loop variable*/
	char pattern[80]; /* the grep pattern string (first|second|third) for the arguments given */
	strcpy (pattern,""); /*initialize the pattern string for concatenation*/
	for (i=1;i<argc-1;i++){
		strcat (pattern, argv[i]); /*concatenate the argument to the pattern string*/
		strcat (pattern, "|"); /*word delimiter required for grep -E*/
	}
	strcat (pattern, argv[argc-1]); /*concatenate the last argument to the pattern*/

	char *printenv[] = {"printenv", NULL}; /* args for printenv*/
	char *sort[] = {"sort", NULL}; /* args for sort */
	char *less[] = {"less", NULL}; /* args for less*/
	char *grep[] = {"grep", "-E", pattern, NULL}; /* args for grep */
	
	int retval = pipe (first_pipe); /*create first pipe */
	if (retval == -1){
		err ("Failed to create first pipe");
	}
	retval = pipe (second_pipe); /* create second pipe */
	if (retval == -1){
		err ("Failed to create second pipe");
	}
	retval = pipe (third_pipe); /* create third pipe */
	if (retval == -1){
		err ("Failed to create third pipe");
	}
	int pid = fork (); /*fork first process */
	if (pid == 0){
		if (useGrep)
			retval = dup2 (first_pipe [WRITE], STDOUT_FILENO); /* redirect STDOUT to the first pipe write end*/
		else
			retval = dup2 (second_pipe[WRITE], STDOUT_FILENO); /* redirect STDOUT to the second pipe write end (skip first pipe) */
		if (retval == -1){
			err ("dup2 failed");
		}
		close_pipes (); /* close all pipe ends */
		retval = execvp (*printenv, printenv); /* execute printenv */
		if (retval == -1)
			err ("execution of printenv failed")
		exit (0); /* exit child process */
	}
	if (useGrep){
		pid = fork ();
		if (pid == 0){
			retval = dup2 (first_pipe [READ], STDIN_FILENO); /* redirect STDIN to first pipe read end*/
			if (retval == -1)
				err ("dup2 failed");
			retval = dup2 (second_pipe [WRITE], STDOUT_FILENO); /*redirect STDOUT to second pipe */
			if (retval == -1)
				err ("dup2 failed");
			close_pipes (); /* close all pipes */
			retval = execvp (*grep, grep); /* execute grep with parameters */
			if (retval == -1)
				err ("execution of grep failed")
			exit (0); /* exit child process */
		}
	}
	pid = fork ();
	if (pid == 0){
		retval = dup2 (second_pipe [READ], STDIN_FILENO); /* redirect STDIN to second pipe read end */
		if (retval == -1)
			err ("dup2 failed");
		retval = dup2 (third_pipe [WRITE], STDOUT_FILENO); /* redirect STDOUT to third pipe write end */
		if (retval == -1)
			err ("dup2 failed");
		close_pipes (); /* close all pipe ends */
		retval = execvp (*sort, sort); /* execute sort */
		if (retval == -1)
			err ("execution of sort failed")
		exit (0); /*exit child process */
	}
	pid = fork ();
	if (pid == 0){
		retval = dup2 (third_pipe [READ], STDIN_FILENO); /* redirect STDIN to third pipe read end */
		if (retval == -1)
			err ("dup2 failed");
		close_pipes (); /* close all pipe ends */
		retval = execvp (*less, less); /* execute less */
		if (retval == -1)
			err ("execution of less failed");
		exit (0); /* exit from child process */
	}
	close_pipes (); /* close all pipe ends in parent process */
	for (i = 0; i < 4; i++)
		wait(NULL); /* wait for all child processes to end */
	exit (0); /* exit parent process cleanly */
}
