#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>
#include <string.h>
#include <mcrypt.h>

MCRYPT td;
struct termios terminal_saved;
int fd_log;
int LOG_FLAG = 0;
static int ENC_FLAG = 0;

void vanilla_exit(){
    tcsetattr(0, TCSAFLUSH, &terminal_saved);
}

void encrypt_exit(){
    mcrypt_module_close(td);
    tcsetattr(0, TCSAFLUSH, &terminal_saved);
}

//thread 2: read input from the sockfd and write it to stdout.
void * thread_func(void * arg){
    int fd = *((int *) arg);
    char buffer[1];
    int size = 0;
    while((size = read( fd, buffer, 1 )) > 0){
        if (buffer[0] == '\004'){
            close(fd);
            exit(1);
        }
        else{
            if (LOG_FLAG == 1){
                write(fd_log, "RECEIVED 1 bytes: ", 18);
                write(fd_log, buffer, size);
                write(fd_log, "\n", 1);
            }
            if (ENC_FLAG)
                mdecrypt_generic (td, buffer, 1);
            write(1, buffer, size);
        }
    }
    //eof case
    if (size == 0){
        close(fd);
        exit(1);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    int sockfd, portno;

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
            case 'p': 
                portno = atoi(optarg);
                break;

            case 'l': 
                fd_log = open(optarg, O_CREAT | O_WRONLY | O_TRUNC, 0600); 
                LOG_FLAG = 1;
                break;

            default:
                break;
        }
    }

    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    //open socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server = gethostbyname("localhost");
    memset((char *) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy((char *)&serv_addr.sin_addr.s_addr,
        (char *)server->h_addr, 
         server->h_length);
    serv_addr.sin_port = htons(portno);

    //terminal settings
    struct termios terminal_modded;
    tcgetattr(0, &terminal_modded);
    tcgetattr(0, &terminal_saved);

    terminal_modded.c_lflag &= ~(ICANON | ECHO);
    terminal_modded.c_cc[VTIME] = 0;
    terminal_modded.c_cc[VMIN] = 1;
    tcsetattr(0, TCSAFLUSH, &terminal_modded);

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
    else {
        //set normal exit func
        atexit(vanilla_exit);
    }

    //connection
    connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));

    //thread 2: recieveing sockfd thread
    pthread_t sock_reader;
    if(pthread_create(&sock_reader, NULL, &thread_func, &sockfd)) {
        perror("pthread problem\n");
        exit(0);
    }

    //thread 1: main thread
    //read input from the keyboard, echo it to stdout, and forward it to the sockfd 
    int size;
    char buffer[1];

    while((size = read( 0, buffer, 1 )) > 0){
        if (buffer[0] == '\004'){
            close(sockfd);
            exit(0);
        }
        else{
            write(1, buffer, size);
            if (ENC_FLAG)
                mcrypt_generic (td, buffer, 1);
            write(sockfd, buffer, size);
            if (LOG_FLAG == 1){
                write(fd_log, "SENT 1 bytes: ", 14);
                write(fd_log, buffer, size);
                write(fd_log, "\n", 1);
            }
        }
    }

    return 0;
}