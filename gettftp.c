#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

int main(int argc,char *argv[]){

    if (argc != 4) {//Error if not enough args
       fprintf(stderr, "Usage: %s <host> <port> <filename>\n", argv[0]);
       return 1;
   }

    //Question 1 : (get the arguments)
    const char *host = argv[1];
    const char *port = argv[2];
    const char *filename = argv[3];

    //Set hints to get UDP server only with TCP and IPV4 
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    //Question 2 :
    if (getaddrinfo(host, port, &hints, &res) != 0) { 
       perror("Erreur de résolution DNS");
       exit(1);
   }


    //Question 3 :
    int sock;

    //If the socket isnt a connection IPV4 and TCP
    if (sock = socket(PF_INET,SOCK_STREAM,IPPROTO_TCP)<0){
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    //Verify that we found the only one UDP server
    struct addrinfo *current;
    for (current = res; current != NULL; current = current->ai_next) {
       printf("Famille : %d, Type : %d, Protocole : %d\n",current->ai_family, current->ai_socktype, current->ai_protocol);
   }
}