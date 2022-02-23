/* udp_client.c */ 
/* Programmed by Adarsh Sethi */
/* Sept. 19, 2021 */

#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset, memcpy, and strlen */
#include <netdb.h>          /* for struct hostent and gethostbyname */
#include <sys/socket.h>     /* for socket, sendto, and recvfrom */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */
#define STRING_SIZE 1024



struct server_packet {
unsigned short request_ID;
unsigned short sequence_num;
unsigned short last;
unsigned short count;
uint16_t payload[25];
};


int main(void) {

   int sock_client;  /* Socket used by client */ 

   struct sockaddr_in client_addr;  /* Internet address structure that
                                        stores client address */
   unsigned short client_port;  /* Port number used by client (local port) */

   struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
   struct hostent * server_hp;      /* Structure to store server's IP
                                        address */
   char server_hostname[STRING_SIZE]; /* Server's hostname */
   unsigned short server_port=0;  /* Port number used by server (remote port) */

   char sentence[STRING_SIZE];  /* send message */
   char modifiedSentence[STRING_SIZE]; /* receive message */
   unsigned int msg_len;  /* length of message */
   int bytes_sent, bytes_recd; /* number of bytes sent or received */
  
   /* open a socket */

   if ((sock_client = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
      perror("Client: can't open datagram socket\n");
      exit(1);
   }

   /* Note: there is no need to initialize local client address information
            unless you want to specify a specific local port.
            The local address initialization and binding is done automatically
            when the sendto function is called later, if the socket has not
            already been bound. 
            The code below illustrates how to initialize and bind to a
            specific local port, if that is desired. */

   /* initialize client address information */

   client_port = 0;   /* This allows choice of any available local port */

   /* Uncomment the lines below if you want to specify a particular 
             local port: */
   /*
   printf("Enter port number for client: ");
   scanf("%hu", &client_port);
   */

   /* clear client address structure and initialize with client address */
   memset(&client_addr, 0, sizeof(client_addr));
   client_addr.sin_family = AF_INET;
   client_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* This allows choice of
                                        any host interface, if more than one 
                                        are present */
   client_addr.sin_port = htons(client_port);

   /* bind the socket to the local client port */

   if (bind(sock_client, (struct sockaddr *) &client_addr,
                                    sizeof (client_addr)) < 0) {
      perror("Client: can't bind to local address\n");
      close(sock_client);
      exit(1);
   }

   /* end of local address initialization and binding */

   /* initialize server address information */

   printf("Enter hostname of server: ");
   scanf("%s", server_hostname);
   if ((server_hp = gethostbyname(server_hostname)) == NULL) {
      perror("Client: invalid server hostname\n");
      close(sock_client);
      exit(1);
   }

while( server_port<=0 || server_port>65535){
   printf("Enter port number for server: ");
   scanf("%hu", &server_port);
}

char run_again[STRING_SIZE] = "y"; // char to check if the user wants to send another request

while(strcmp(run_again,"y")==0){
unsigned int count_request=0; // variable to store the amount of integers requested

while( count_request <=0 || count_request >65535){
	printf("Enter the count of how many intergers you would like to request from the server: ");
	scanf("%d", &count_request);
}







   /* Clear server address structure and initialize with server address */
   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   memcpy((char *)&server_addr.sin_addr, server_hp->h_addr,
                                    server_hp->h_length);
   server_addr.sin_port = htons(server_port);



   /* send message */
  
   bytes_sent = sendto(sock_client, &count_request, sizeof(int), 0,
            (struct sockaddr *) &server_addr, sizeof (server_addr));

   /* get response from server */
struct server_packet spkt;
unsigned int request_ID; // request ID from the server
int total_pkts=0; // total packets recieved
int total_bytes=0; // total bytes recieved
int sequence_sum=0; // sum of sequence numbers
long checksum=0; // checksum of all data ints recieved

 /* To initally get the request_ID */
bytes_recd = recvfrom(sock_client,&spkt, sizeof(struct server_packet), 0,
	(struct sockaddr *) 0, (int *)0);

request_ID=spkt.request_ID; // initially retrieve the request ID to check later
total_bytes += bytes_recd;
total_pkts += 1;
sequence_sum+=spkt.sequence_num;

for(int i=0;i<25;i++){
checksum += ntohs(spkt.payload[i]); // add data ints to checksum
}
   /* while loop to get the rest of the packets */
while(count_request!=1){ // same as while(true); but this also solves the case when there count requested is set to 1
 bytes_recd = recvfrom(sock_client,&spkt, sizeof(struct server_packet), 0,
                (struct sockaddr *) 0, (int *) 0);
if(request_ID==spkt.request_ID)
{
	total_bytes += bytes_recd;
	total_pkts += 1;
	sequence_sum+=spkt.sequence_num;
	for(int i=0;i<25;i++){
	checksum += ntohs(spkt.payload[i]); // add data ints to checksum
	}
}

if(spkt.last==1){ // case for when recieved the last packet
break;
}

}
printf("\n--------------------------------------------------------------\n");
printf("Request ID: %d\nCount: %d\nRecieved bytes: %d\nRecieved packets: %d\nSequence number sum: %d\nChecksum: %ld\n",
request_ID,count_request,total_bytes, total_pkts,sequence_sum,checksum);
printf("--------------------------------------------------------------\n");
printf("Would you like to send another request? y/n: ");
scanf("%s",run_again);


while(strcmp(run_again,"y")!=0 && strcmp(run_again,"n")!=0){
printf("Would you like to send another request? y/n: ");
scanf("%s",run_again);
}


}
   /* close the socket */

   close (sock_client);
}

