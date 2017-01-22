#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <mcrypt.h>
#include <fcntl.h>

static int ENC_FLAG = 0;
MCRYPT td;

void handler(int signum){
	if (signum == SIGPIPE){
		close(0);
		exit(2);
	}
}

void encrypt_exit(){
    mcrypt_module_close(td);
}

struct thread_info{
	int fd;
	int fd_for_close;
	int pid;
};

//thread 2: read input from the shell pipe and write it to sockfd(fd 1)
void * thread_func(void * arg){
	int fd = ((struct thread_info *) arg)->fd;
	int fd_close = ((struct thread_info *) arg)->fd_for_close;
	int pid = ((struct thread_info *) arg)->pid;
	char buffer[1];
	int size = 0;
	while((size = read( fd, buffer, 1 )) > 0){
		if (buffer[0] == '\004'){
			close(0);
			close(fd_close);
			kill(pid, SIGINT); //send a SIGINT to the shell
			exit(2);
		}
		else{
			if (ENC_FLAG)
                mcrypt_generic (td, buffer, 1);
			write(1, buffer, size);
		}
	}
	//Case: exit()
	if(size == 0){
		write(1, '\004', 1);
		close(0);
		close(fd_close);
		kill(pid, SIGINT); //send a SIGINT to the shell
		exit(2);
	}
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, clilen;

    while(1){
    	static struct option long_options[] = {
        	{"port", required_argument, 0, 'p'},
            {"log", required_argument, 0, 'l'},
            {"encrypt", no_argument, &ENC_FLAG, 1},
            {0, 0, 0, 0}
        };

        int option_index = 0;

        int c = getopt_long(argc, argv, "abc:d:012", long_options, &option_index);

        if (c == -1)
            break;

        switch(c){
            case 'p': //open file 
                portno = atoi(optarg);
                break;

            default:
                break;
        }
    }

    //set up encryption and exit func
    if (ENC_FLAG == 1){
        int i;
        char *key;
        char *IV;
        int keysize=16; /* 128 bits */
        key=calloc(1, keysize);
        int fd_key = open("my.key", O_RDONLY);
        read(fd_key, key, keysize);

        td = mcrypt_module_open("twofish", NULL, "cfb", NULL);
        if (td==MCRYPT_FAILED) {
            return 1;
        }
        IV = malloc(mcrypt_enc_get_iv_size(td));
        for (i=0; i< mcrypt_enc_get_iv_size( td); i++) {
            IV[i]=rand();
        }
        i=mcrypt_generic_init( td, key, keysize, IV);
        if (i<0) {
            mcrypt_perror(i);
            return 1;
        }
        atexit(encrypt_exit);
    }

    struct sockaddr_in serv_addr, cli_addr;
    
    //open socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    //listen for client
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    
    //fork a child process which will exec a shell to process commands received; the server process should communicate with the shell via pipes
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		
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

    //FORK
    int pid = fork();

    //CHILD (shell process)
    if (pid == 0) {
      	//redirect the shell process’s stdin/stdout/stderr to the appropriate pipe ends 
        close(pipe_from_shell[0]); //closes the read end of pfs
        dup2(pipe_from_shell[1], 1); // stdout duped to write end of pfs 

        close(pipe_to_shell[1]); //closes the write end of pts
        dup2(pipe_to_shell[0], 0); //stdin duped to read end of pts

        execl("/bin/bash", "/bin/bash", NULL);
    }
    //PARENT (server process)
    else{
        //setup signal handler
        signal(SIGPIPE, handler);

        //close unused pipes
        close(pipe_from_shell[1]); //closes the write end of pfs
        close(pipe_to_shell[0]); //closes the read end of pts

        //redirect the server process’s stdin/stdout/stderr to the socket
        dup2(newsockfd, 0);
        dup2(newsockfd, 1);
        dup2(newsockfd, 2);

   		//thread 2: shell messenger thread
   		//executes thread_func'
   		struct thread_info tinfo;
   		tinfo.fd = pipe_from_shell[0];
   		tinfo.pid = pid;
   		tinfo.fd_for_close = sockfd;
        pthread_t shell_messenger;
        if(pthread_create(&shell_messenger, NULL, &thread_func, &tinfo)) {
			perror("pthread problem\n");
			exit(0);
		}

        //thread 1: main thread
        //receive the client’s commands and send them to the shell
		char buffer[1];
		int size = 0;
		while((size = read( 0, buffer, 1 )) > 0){
			if (ENC_FLAG)
                mdecrypt_generic (td, buffer, 1);
			write(pipe_to_shell[1], buffer, size);
		}
		if (size == 0){
			close(newsockfd);
			close(sockfd);
			kill(pid, SIGINT); //send a SIGINT to the shell
			exit(1);
		}
	}
}