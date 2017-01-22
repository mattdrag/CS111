#include <unistd.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>

struct termios terminal_saved;
int EOF_FROM_SHELL = 0;


void vanilla_exit(){
	tcsetattr(0, TCSAFLUSH, &terminal_saved);
}

void shell_exit(){
	tcsetattr(0, TCSAFLUSH, &terminal_saved);
	int status;
	waitpid(-1, &status, 0);
	if(WIFEXITED(status)){
		perror("Child process ended normally.\n");
		printf("Exit status: %d\n", WEXITSTATUS(status));
	}
	else
		perror("Child process ended abnormally.\n");
}

void EOF_exit(){
	tcsetattr(0, TCSAFLUSH, &terminal_saved);
}

void handler(int signum){
	if (signum == SIGPIPE){
		perror("Recieved SIGPIPE.");
		exit(1);
	}
	exit(1);
}

//thread 2: read input from the shell pipe and write it to stdout.
void * thread_func(void * fd){
	int * fd_useable = (int *) fd;
	char buffer[1];
	int size = 0;
	while((size = read( *fd_useable, buffer, 1 )) > 0){
		if (buffer[0] == '\004'){
			EOF_FROM_SHELL = 1;
		}
		else
			write(1, buffer, size);
	}
	return NULL;
}

int main(int argc, char *argv[]){

	static int shell = 0;

	while(1){
		static struct option long_options[] = {
			{"shell", no_argument, &shell, 1},
			{0, 0, 0, 0}
		};

		int option_index = 0;

		int c = getopt_long(argc, argv, "s", long_options, &option_index);

		if (c == -1)
			break;
	}

    //terminal settings
	struct termios terminal_modded;
	tcgetattr(0, &terminal_modded);
	tcgetattr(0, &terminal_saved);

	terminal_modded.c_lflag &= ~(ICANON | ECHO);
	terminal_modded.c_cc[VTIME] = 0;
	terminal_modded.c_cc[VMIN] = 1;
	tcsetattr(0, TCSAFLUSH, &terminal_modded);


	//////////////////
	////LAB PART 2////
	//////////////////
	if (shell){
		//handle ctrl-c
		terminal_modded.c_lflag &= ~(ISIG);
		tcsetattr(0, TCSAFLUSH, &terminal_modded);

		//set atexit function to shell
		int i = atexit(shell_exit);
    	if (i != 0) {
    		perror("cannot set exit function\n");
    		tcsetattr(0, TCSAFLUSH, &terminal_saved);
    		exit(1);
    	}

		//set up the pipes
		int pipe_from_shell[2];
		int pipe_to_shell[2];
		if (pipe(pipe_from_shell) == -1){
			perror("pipe error\n");
            exit(1);
        }
		if (pipe(pipe_to_shell) == -1){
			perror("pipe error\n");
            exit(1);
        }


        //FORK//
        int pid = fork();
        //CHILD (shell process)
        if (pid == 0){  
        	close(pipe_from_shell[0]); //closes the read end of pfs
        	dup2(pipe_from_shell[1], 1); // stdout duped to write end of pfs 

        	close(pipe_to_shell[1]); //closes the write end of pts
        	dup2(pipe_to_shell[0], 0); //stdin duped to read end of pts

        	execl("/bin/bash", "/bin/bash", NULL);
        }

        //PARENT (terminal process)
        else{
        	//setup signal handler
        	signal(SIGPIPE, handler);

        	close(pipe_from_shell[1]); //closes the write end of pfs
        	close(pipe_to_shell[0]); //closes the read end of pts

   			//thread 2: shell messenger thread
        	pthread_t shell_messenger;
        	if(pthread_create(&shell_messenger, NULL, &thread_func, &pipe_from_shell[0])) {
				perror("pthread problem\n");
				exit(0);
			}

        	//thread 1: main thread
        	//read input from the keyboard, echo it to stdout, and forward it to the shell 
			char buffer[1];
			char crlf[2] = {0x0D, 0x0A};
			char lf[1] = {0x0A};
			int size = 0;
			while((size = read( 0, buffer, 1 )) > 0){
				if(EOF_FROM_SHELL){
					kill(pid, SIGINT); //send a SIGINT to the shell
					exit(1);
				}
				else if (buffer[0] == '\004'){
					//close the pipe, send a SIGHUP to the shell, restore terminal modes, and exit with return code 0
					close(pipe_from_shell[0]); //closes the read end of pfs
        			close(pipe_to_shell[1]); //closes the write end of pts
					kill(pid, SIGHUP); 
					exit(0);
				}
				else if (buffer[0] == '\003'){
					kill(pid, SIGINT); //send a SIGINT to the shell
				}
				else if (buffer[0] == '\015' || buffer[0] == '\012'){
					write(1, crlf, sizeof(crlf));
					write(pipe_to_shell[1], lf, sizeof(lf));
				}
				else{
					write(1, buffer, size);
					write(pipe_to_shell[1], buffer, size);
				}
			}
        }
	}

	//////////////////
	////LAB PART 1////
	//////////////////
	else{
		//set atexit function to vanilla
		int i = atexit(vanilla_exit);
    	if (i != 0) {
    		perror("cannot set exit function\n");
    		tcsetattr(0, TCSAFLUSH, &terminal_saved);
    		exit(1);
    	}

		//read/writes
		char buffer[1];
		char crlf[2] = {0x0D, 0x0A};
		int size = 0;
		while((size = read( 0, buffer, 1 )) > 0){
			if (buffer[0] == '\015' || buffer[0] == '\012')
				write(1, crlf, sizeof(crlf));
			else if (buffer[0] == '\004')
				exit(1);
			else
				write(1, buffer, size);
		}
	}
	return 0;
}