#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>

#define READ ( 0 ) /* the index of the read end of the file descriptor arrays filled by pipe (1)*/
#define WRITE ( 1 ) /* the index of the write end of the file descriptor arrays filled by pipe(1)*/

int first_pipe[2]; /* the pipe between he printenv_args and grep_args processes. Unused if program is strted without arguments */
int second_pipe[2]; /* the pipe between the grep_args and sort_args processes. If grep_args is not to be called, this pipe connects the printenv_args and sort_args processes */
int third_pipe[2]; /* the pipe between the sort_args and pager_args processes */

/* prints the specified string to STDERR*/
void err (char * msg){
	perror (msg);
	//fprintf (stderr, "%s",msg);
	exit (1);
}

/* close_pipes
* close_pipes calls close (1) on both pipe ends of all three global pipes
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
/**
* Main entry for digenv. Runs the command printenv | grep parameters | sort | $PAGER
* if the PAGER environment variable is not set, digenv uses less instead, and
* if less is undefined, uses more as the pager.
*/
int main(int argc, char *argv[], char *envp[])
{
	char *pager;
	pager = getenv ("PAGER"); /* get PAGER environemnt variable */
	if (pager == NULL)
		pager = "less"; /* use less if PAGER is undefined */
	bool usegrep_args = true; /* used to indicate whether grep_args should be called*/
	if (argc < 2) /*find out whether any arguments were used (i.e. grep_args should be called) */
	usegrep_args = false;
	int i; /* loop variable*/
	char pattern[80]; /* the grep_args pattern string (first|second|third) for the arguments given */
	strcpy (pattern,""); /*initialize the pattern string for concatenation*/
	for (i=1;i<argc-1;i++){
		strcat (pattern, argv[i]); /*concatenate the argument to the pattern string*/
		strcat (pattern, "|"); /*word delimiter required for grep_args -E*/
	}
	strcat (pattern, argv[argc-1]); /*concatenate the last argument to the pattern*/

	char *printenv_args[] = {"printenv", NULL}; /* arguments for printenv*/
	char *sort_args[] = {"sort", NULL}; /* arguments for sort */
	char *pager_args[] = {pager, NULL}; /* arguments for pager*/
	char *grep_args[] = {"grep", "-E", pattern, NULL}; /* arguments for grep */
	
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
		if (usegrep_args)
			retval = dup2 (first_pipe [WRITE], STDOUT_FILENO); /* redirect STDOUT to the first pipe write end*/
		else
			retval = dup2 (second_pipe[WRITE], STDOUT_FILENO); /* redirect STDOUT to the second pipe write end (skip first pipe) */
		if (retval == -1){
			err ("dup2 failed");
		}
		close_pipes (); /* close all pipe ends */
		retval = execvp (*printenv_args, printenv_args); /* execute printenv_args */
		if (retval == -1)
			err ("execution of printenv_args failed");
		exit (0); /* exit child process */
	}
	if( -1 == pid ) /* fork() misslyckades */
 		err( "Cannot fork()" );
	if (usegrep_args){
		pid = fork ();
		if (pid == 0){
			retval = dup2 (first_pipe [READ], STDIN_FILENO); /* redirect STDIN to first pipe read end*/
			if (retval == -1)
				err ("dup2 failed");
			retval = dup2 (second_pipe [WRITE], STDOUT_FILENO); /*redirect STDOUT to second pipe */
			if (retval == -1)
				err ("dup2 failed");
			close_pipes (); /* close all pipes */
			retval = execvp (*grep_args, grep_args); /* execute grep_args with parameters */
			if (retval == -1)
				err ("execution of grep_args failed");
			exit (0); /* exit child process */
		}
		if( -1 == pid ) /* fork() misslyckades */
 			{ perror( "Cannot fork()" ); exit( 1 ); }
	}
	pid = fork (); /* fork process executing sort_args */
	if (pid == 0){
		retval = dup2 (second_pipe [READ], STDIN_FILENO); /* redirect STDIN to second pipe read end */
		if (retval == -1)
			err ("dup2 failed");
		retval = dup2 (third_pipe [WRITE], STDOUT_FILENO); /* redirect STDOUT to third pipe write end */
		if (retval == -1)
			err ("dup2 failed");
		close_pipes (); /* close all pipe ends */
		retval = execvp (*sort_args, sort_args); /* execute sort_args */
		if (retval == -1)
			err ("execution of sort_args failed");
		exit (0); /*exit child process */
	}
	if( -1 == pid ) /* fork() misslyckades */
		err( "Cannot fork()" );
	pid = fork ();
	if (pid == 0){
		retval = dup2 (third_pipe [READ], STDIN_FILENO); /* redirect STDIN to third pipe read end */
		if (retval == -1)
			err ("dup2 failed");
		close_pipes (); /* close all pipe ends */
		retval = execvp (*pager_args, pager_args); /* execute pager*/
		if (retval == -1){
			pager_args[0] = "more";
			retval = execvp (*pager_args, pager_args); /* try executing more instead */
		}
		if (retval == -1)
			err ("execution of pager failed");
		exit (0); /* exit from child process */
	}
	if( -1 == pid ) /* fork() misslyckades */
 		{ perror( "Cannot fork()" ); exit( 1 ); }
	close_pipes (); /* close all pipe ends in parent process */
	for (i = 0; i < 4; i++)
		wait(NULL); /* wait for all child processes to end */
	exit (0); /* exit parent process cleanly */
}
