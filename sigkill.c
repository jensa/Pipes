#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

int sigkilled;

static void kill_handler (int sig){
	if (sigkilled < 10){
	printf ("ignoring ");
	sigkilled += 1;
	}else{
		exit (0);
	}
}

int main(int argc, char const *argv[])
{
	sigkilled = 0;
	signal(SIGINT, kill_handler);
	while (1){
		sleep (1);
		printf("%s\n","this" );
	};
	return 0;
}

