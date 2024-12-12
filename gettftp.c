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
       if (argc != 4) { //Verify the arguments
        fprintf(stderr, "Usage %s <host> <port> <filename>\n",argv[0]);
        exit(EXIT_FAILURE); 
    }

    //Question 1 : (get the arguments)
    const char *host = argv[1];
    const char *port = argv[2];
    const char *file_name = argv[3];

    //Set hints to get UDP server only with IPV4
    struct addrinfo hints, *res;
    memset(&hints,0,sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    //Question 2 :
    if (getaddrinfo(argv[1],argv[2],&hints, &res) != 0){ //Verify if error while using getaddrinfo
        perror("Error while getaddrinfo");
        exit(EXIT_FAILURE);
    };

    //Verify that we found the only one UDP server
    struct addrinfo *current = res;
    while (current->ai_next != NULL){
        printf("Found one result");
        printf("\t ai_family = %d\n",current->ai_family);
        current = current->ai_next;
        }
}