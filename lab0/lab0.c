#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <sys/stat.h> 
#include <fcntl.h>

void handler(int signum){
	perror("There was a segfault, but we caught it for you!\n");
	exit(3);
}

void myfault(){
	int *a = NULL;
	int b = *a;
}

int main(int argc, char *argv[]){

	static int segfault = 0;
	static int catch = 0;
	int fd0;
	int fd1;

	while(1){
		static struct option long_options[] = {
			{"input", required_argument, 0, 'i'},
			{"output", required_argument, 0, 'o'},
			{"segfault", no_argument, &segfault, 1},
			{"catch", no_argument, &catch, 1},
			{0, 0, 0, 0}
		};

		int option_index = 0;

		int c = getopt_long(argc, argv, "abc:d:012", long_options, &option_index);

		if (c == -1)
			break;

		switch(c){
			case 'i': //open file 
            	fd0 = open(optarg, O_RDONLY);
            	if(fd0 == -1){
            		perror("input file failure");
            		exit(1);
            	}
            	dup2(fd0,0);
            	close(fd0);
				break;
			case 'o':
            	fd1 = open(optarg, O_CREAT | O_WRONLY | O_TRUNC, 0600);
            	if(fd1 == -1){
            		perror("output file failure");
            		exit(2);
            	}
            	dup2(fd1,1);
            	close(fd1);
				break;

			case 0:
           		break;
			
			default:
				break;
		}
	}

	if (catch){
		signal(SIGSEGV, handler);
	}

	if (segfault){
		myfault();
	}

	char buffer[1];
	while (read(0, buffer, 1) > 0){
		write(1, buffer, 1);
	}

	return 0;
}