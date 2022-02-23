/* udp_server.c */
/* Programmed by Adarsh Sethi */
/* Sept. 19, 2021 */

#include <ctype.h>          /* for toupper */
#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset */
#include <sys/socket.h>     /* for socket, sendto, and recvfrom */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */

#define STRING_SIZE 1024

/* SERV_UDP_PORT is the port number on which the server listens for
   incoming messages from clients. You should change this to a different
   number to prevent conflicts with others in the class. */

#define SERV_UDP_PORT 65100

struct server_packet{
unsigned short request_ID; // request ID
unsigned short sequence_num; // sequence number
unsigned short last; // variable for last packet
unsigned short count;
uint16_t payload[25]; //store the 25 Data integers

};

struct client_packet{
unsigned short request_ID;
unsigned short count;
};


int main(void) {

   int sock_server;  /* Socket on which server listens to clients */

   struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
   unsigned short server_port;  /* Port number used by server (local port) */

   struct sockaddr_in client_addr;  /* Internet address structure that
                                        stores client address */
   unsigned int client_addr_len;  /* Length of client address structure */

   char sentence[STRING_SIZE];  /* receive message */
   char modifiedSentence[STRING_SIZE]; /* send message */
   unsigned int msg_len;  /* length of message */
   int bytes_sent, bytes_recd; /* number of bytes sent or received*/
   int count_request; // count request from the client
   unsigned int i;  /* temporary loop variable */

   /* open a socket */

   if ((sock_server = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
      perror("Server: can't open datagram socket\n");
      exit(1);
   }

   /* initialize server address information */

   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   server_addr.sin_addr.s_addr = htonl (INADDR_ANY);  /* This allows choice of
                                        any host interface, if more than one
                                        are present */
   server_port = SERV_UDP_PORT; /* Server will listen on this port */
   server_addr.sin_port = htons(server_port);

   /* bind the socket to the local server port */

   if (bind(sock_server, (struct sockaddr *) &server_addr,
                                    sizeof (server_addr)) < 0) {
      perror("Server: can't bind to local address\n");
      close(sock_server);
      exit(1);
   }

   /* wait for incoming messages in an indefinite loop */

   printf("Waiting for incoming messages on port %hu\n\n", 
                           server_port);

   client_addr_len = sizeof (client_addr);

int server_requestID=1;

   for (;;) {
int client_request;
   count_request = recvfrom(sock_server, &client_request, sizeof(int), 0,
	(struct sockaddr *) &client_addr, &client_addr_len);
 

struct server_packet spkt;
spkt.last=0; // assign last to 0
spkt.sequence_num=0; // assign sequence number to 0
int total_pkts=0; // total packets transmitted
int total_bytes=0; // total bytes transmitted
int sequence_sum=0; // sum of sequence numbers
long checksum=0; // checksum of all data ints transmitted

spkt.request_ID=server_requestID;
spkt.count=client_request;
/*     printf("Server request ID is: %d\n     Count is %d\n\n",
                         spkt.request_ID, spkt.count);
*/

for(int i=0;i<client_request;i++){ // loop through the client's request

for(int j=0; j<25;j++){
spkt.payload[j]=htons(rand()); // generates a random int and stores it into the payload
checksum+=htons(spkt.payload[j]);
}


spkt.sequence_num +=1;

      /* send message */
      if(i==(client_request-1)){
       spkt.last=1; // assign 1 to the last packet
      }
 
      bytes_sent = sendto(sock_server, &spkt, sizeof(struct server_packet), 0,
               (struct sockaddr*) &client_addr, client_addr_len);

     total_bytes += bytes_sent;
     total_pkts +=1;
     sequence_sum +=spkt.sequence_num;
}// end client_request for loop
printf("-------------------------------------------------------------\n");
printf("Request ID: %d\nCount: %d\nTransmited bytes: %d\nTransmitted packets: %d\nSequence number sum: %d\nChecksum: %ld\n",
spkt.request_ID,spkt.count,total_bytes, total_pkts,sequence_sum,checksum);
printf("-------------------------------------------------------------\n");
server_requestID+=1;
   }
}
