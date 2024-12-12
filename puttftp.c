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
    if (argc != 4) {
        fprintf(stderr, "Usage %s <host> <port> <filename>\n",argv[0]);
        exit(EXIT_FAILURE); 
    }

    const char *host = argv[1];
    const char *port = argv[2];
    const char *file_name = argv[3];

    struct addrinfo hints, *res;
    memset(&hints,0,sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    getaddrinfo(argv[1],argv[2],&hints, &res);
    
    struct addrinfo *current = res;
    while (current->ai_next != NULL){
        printf("Found one result :\n");
        printf("\t ai_family = %d",current->ai_family);
        printf("\t ai_socktype = %d",current->ai_socktype);
        printf("\t ai_protocol = %d",current->ai_protocol);
        printf("\n\r");

        char string_target[128] = {0};
        char string_server[128] = {0};
        getnameinfo(current->ai_addr,current->ai_addrlen, string_target,128,string_server,128,NI_NUMERICHOST|NI_NUMERICSERV);
        printf("\t host : %s\n",string_target);
        printf("\t server : %s\n",string_server);

        current = current->ai_next;
    }
}