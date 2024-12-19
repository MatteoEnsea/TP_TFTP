
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

#define BUF_SIZE 516  // 512 bytes data + 4 bytes header
#define mode "octet"
#define DEFAULT_BLOCKSIZE 512
#define TIMEOUT 5  

//Opcode constants
enum TFTP_OPCODE {
   RRQ = 1,  //512 bytes data + 4 bytes header
   WRQ,      
   DATA,     
   ACK,      
   ERROR     
};

//================================== Functions =========================================================

int create_socket(struct addrinfo *res) {
   int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
   if (sockfd < 0) {
       perror("Error creating socket");
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

int receive_ack(int sockfd, struct sockaddr *server_addr, socklen_t *addrlen, int expected_block) {
    char buffer[BUF_SIZE];
    int len = recvfrom(sockfd, buffer, BUF_SIZE, 0, server_addr, addrlen);

    if (len < 0) {//If error in the reception
        perror("Error receiving ACK");
        exit(1);
    }

    if (buffer[1] != ACK) {//If no ACK in the received packet
        printf("Unexpected packet received (opcode %d)\n", buffer[1]);
        exit(1);
    }

    int received_block = (buffer[2] << 8) | buffer[3];//Shift the received packet to keep message after the ACK packet
    if (received_block != expected_block) {
        printf("Unexpected ACK: expected %d, received %d\n", expected_block, received_block);
        exit(1);
    }

    return len;
}

void send_data(int sockfd, struct sockaddr *server_addr, socklen_t addrlen, const char *data, int data_len, int block_number) {
   char buffer[BUF_SIZE];

   buffer[0] = 0x00;//Size of the packet
   buffer[1] = DATA;
   buffer[2] = block_number >> 8;      //High byte of block number
   buffer[3] = block_number & 0xFF;    //Low byte of block number
   memcpy(&buffer[4], data, data_len); //Copy data into the buffer

   if (sendto(sockfd, buffer, data_len + 4, 0, server_addr, addrlen) < 0) {//If error while senging datas
       perror("Error sending DATA packet");
       exit(1);
   }
}

//========================================== Main ======================================================
int main(int argc, char *argv[]) {
   if (argc != 4) {
       fprintf(stderr, "Usage: %s <host> <port> <filename>\n", argv[0]);
       return 1;
   }

   const char *host = argv[1];      
   const char *port = argv[2];      
   const char *filename = argv[3];  

  //Set hints to get UDP server only on IPV4 
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;

   if (getaddrinfo(host, port, &hints, &res) != 0) {
       perror("DNS resolution error");
       exit(1);
   }

   int sockfd = create_socket(res);

   send_request(sockfd, res, filename, WRQ);

   FILE *file = fopen(filename, "rb");
   if (!file) {
       perror("Error opening file");
       exit(1);
   }

   char buffer[DEFAULT_BLOCKSIZE];
   struct sockaddr server_addr;
   socklen_t server_addrlen = sizeof(server_addr);
   int block_number = 1;

   while (1) {
       //Read a block of data from the file
       int read_size = fread(buffer, 1, DEFAULT_BLOCKSIZE, file);

       //Send the data block
       send_data(sockfd, &server_addr, server_addrlen, buffer, read_size, block_number);

       //Receive the corresponding ACK
       receive_ack(sockfd, &server_addr, &server_addrlen, block_number);

       
       if (read_size < DEFAULT_BLOCKSIZE) {//End of the file
           printf("End of file sent\n");
           break;
       }

       block_number++; 
   }

   fclose(file);      
   freeaddrinfo(res);  
   close(sockfd);      

   return 0;
}