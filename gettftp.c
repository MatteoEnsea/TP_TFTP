#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#define BUF_SIZE 516 //512 bytes data + 4 bytes header
#define mode "octet"
#define DEFAULT_BLOCKSIZE 512
#define TIMEOUT 5

//Opcode constants
enum TFTP_OPCODE {
 RRQ = 1,//RRQ = 1, WRQ = 2, etc.
 WRQ,
 DATA,
 ACK,
 ERROR
};

//================================== Functions =========================================================

int create_socket(struct addrinfo *res) {
 int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

 if (sockfd < 0) {
     perror("Erreur de création du socket");
     exit(1);
 }
 return sockfd;
}

void send_request(int sock, struct addrinfo *res, const char *filename, int op_code) {
 char buffer[BUF_SIZE];
 int len = 0;

 buffer[len++] = 0x00;
 buffer[len++] = op_code; //Opcode (RRQ=1, WRQ=2)
 strcpy(&buffer[len], filename);
 len += strlen(filename) + 1;  //Add null terminator
 strcpy(&buffer[len], mode);
 len += strlen(mode) + 1;      //Add null terminator (again)

 if (sendto(sock, buffer, len, 0, res->ai_addr, res->ai_addrlen) < 0) {
     perror("Erreur lors de l'envoi de la requête");
     exit(1);
 }
}

void send_ack(int sock, struct sockaddr *server_addr, socklen_t addrlen, int block_number) {
 char ack_packet[4]; //Size of ACK packet
 //Fill ACK packet
 ack_packet[0] = 0x00;
 ack_packet[1] = ACK;
 ack_packet[2] = block_number >> 8;
 ack_packet[3] = block_number & 0xFF;

 if (sendto(sock, ack_packet, 4, 0, server_addr, addrlen) < 0) {//If error while sending ACK packet
     perror("Erreur lors de l'envoi de l'ACK");
     exit(1);
 }
}

//Function to receive a packet with a timeout define above
int receive_with_timeout(int sockfd, char *buffer, struct sockaddr *server_addr, socklen_t *addrlen) {
    fd_set read_fds;
    struct timeval timeout;

    //Initialize the file descriptor
    FD_ZERO(&read_fds);
    FD_SET(sockfd, &read_fds);

    //Set timeout duration
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;

    //Wait for the socket to become ready for reading 
    int ready = select(sockfd + 1, &read_fds, NULL, NULL, &timeout);
    if (ready < 0) {
        perror("Error receiving data");
        exit(1);
    } else if (ready == 0) {//Return -1 for error if timeout reached
        printf("Timeout expired\n");
        return -1; // Timeout occurred
    }
    //Return the received packet if no error
    return recvfrom(sockfd, buffer, BUF_SIZE, 0, server_addr, addrlen);
}


//========================================== Main ======================================================

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
  int sock = create_socket(res);

  //Question 4
  send_request(sock, res, filename,RRQ);

  FILE *file = fopen(filename, "wb");
  if (!file) {
      perror("Erreur d'ouverture du fichier");
      exit(1);
  }

  char buffer[BUF_SIZE];
  struct sockaddr server_addr;
  socklen_t server_addrlen = sizeof(server_addr);
  int block_number = 1;

     while (1) {
     int recv_len = receive_with_timeout(sock, buffer, &server_addr, &server_addrlen);
     if (recv_len < 0) {
         printf("Timeout ou erreur lors de la réception\n");
         break;
     }

     if (buffer[1] == DATA) {
         int received_block = (buffer[2] << 8) | buffer[3];
         if (received_block != block_number) {
             printf("Bloc inattendu : attendu %d, reçu %d\n", block_number, received_block);
             break;
         }

         int data_size = recv_len - 4;
         fwrite(&buffer[4], 1, data_size, file);
         send_ack(sock, &server_addr, server_addrlen, block_number);

         if (data_size < DEFAULT_BLOCKSIZE) {
             printf("Fin du fichier reçue\n");
             break;
         }

         block_number++;
     } else if (buffer[1] == ERROR) {
         printf("Erreur reçue : code %d\n", buffer[3]);
         break;
     }
 }

 fclose(file);
 freeaddrinfo(res);
 close(sock);
}  