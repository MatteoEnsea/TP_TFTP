#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <netdb.h>

int main(int argc,char *argv[]){
    if (argc != 3) {
        fprintf(stderr, "Missing argument(s)\n");
        exit(EXIT_FAILURE); 
    }

    struct addrinfo hints;
    struct addrinfo *res;
    memset(&hints,0,sizeof(struct addrinfo));

    getaddrinfo(argv[1],NULL,&hints, &res);
    struct addrinfo *current = res;
}