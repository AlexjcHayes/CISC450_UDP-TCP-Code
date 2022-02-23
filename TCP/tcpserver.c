/* tcpserver.c */
/* Programmed by Adarsh Sethi */
/* Sept. 19, 2021 */

/* have it find the correct index (have some print statements)

-have it create a new template (keep track of the line number and skip it when rewriting)
-copy and alter for client side

*/
#include <ctype.h>      /* for toupper */
#include <stdio.h>      /* for standard I/O functions */
#include <stdlib.h>     /* for exit */
#include <string.h>     /* for memset */
#include <sys/socket.h> /* for socket, bind, listen, accept */
#include <netinet/in.h> /* for sockaddr_in */
#include <unistd.h>     /* for close */

#define STRING_SIZE 1024

/* SERV_TCP_PORT is the port number on which the server listens for
   incoming requests from clients. You should change this to a different
   number to prevent conflicts with others in the class. */

#define SERV_TCP_PORT 48522
#define SERVER_SECRET_CODE 63829 // Server Secret Code
#define SERVER_TRAVEL_LOCATION "Middletown"

#define TXT_FILE_BUFFER 35 // buffer size for a single line in the Visitors.txt

typedef struct clientServerPkt
{

   unsigned short stepNum;        // step number
   unsigned short clientPort;     // client port number
   unsigned short serverPort;     // server port number
   unsigned short serverScrtCode; // server secret code
   char message[STRING_SIZE];

} clientServerPkt;

int main(void)
{

   int sock_server;     /* Socket on which server listens to clients */
   int sock_connection; /* Socket on which server exchanges data with client */

   struct sockaddr_in server_addr; /* Internet address structure that
                                        stores server address */
   unsigned int server_addr_len;   /* Length of server address structure */
   unsigned short server_port;     /* Port number used by server (local port) */

   struct sockaddr_in client_addr; /* Internet address structure that
                                        stores client address */
   unsigned int client_addr_len;   /* Length of client address structure */

   char sentence[STRING_SIZE];         /* receive message */
   char modifiedSentence[STRING_SIZE]; /* send message */
   unsigned int msg_len;               /* length of message */
   int bytes_sent, bytes_recd;         /* number of bytes sent or received */
   unsigned int i;                     /* temporary loop variable */
   int found_client_flag = 0;          // flag for finding client
   FILE *vistTxt;                      /* pointer to Vistor.txt */
   FILE *temp_vistTxt;                 /* pointer to temp Vistor.txt */
   /* open a socket */

   if ((sock_server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
   {
      perror("Server: can't open stream socket");
      exit(1);
   }

   /* initialize server address information */

   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   server_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* This allows choice of
                                        any host interface, if more than one
                                        are present */
   server_port = SERV_TCP_PORT;                     /* Server will listen on this port */
   server_addr.sin_port = htons(server_port);

   /* bind the socket to the local server port */

   if (bind(sock_server, (struct sockaddr *)&server_addr,
            sizeof(server_addr)) < 0)
   {
      perror("Server: can't bind to local address");
      close(sock_server);
      exit(1);
   }

   /* listen for incoming requests from clients */

   if (listen(sock_server, 50) < 0)
   {                                     /* 50 is the max number of pending */
      perror("Server: error on listen"); /* requests that will be queued */
      close(sock_server);
      //      exit(1);
   }
   printf("I am here to listen ... on port %hu\n\n", server_port);

   client_addr_len = sizeof(client_addr);

   /* wait for incoming connection requests in an indefinite loop */

   for (;;)
   {

      sock_connection = accept(sock_server, (struct sockaddr *)&client_addr,
                               &client_addr_len);
      /* The accept function blocks the server until a
                        connection request comes from a client */
      if (sock_connection < 0)
      {
         perror("Server: accept() error\n");
         close(sock_server);
         //         exit(1);
      }
      struct clientServerPkt server_TCP_Pkt;
      /* receive the message */

      bytes_recd = recv(sock_connection, &server_TCP_Pkt, sizeof(struct clientServerPkt), 0);

      if (bytes_recd > 0)
      {
         server_TCP_Pkt.serverPort = server_addr.sin_port;
         unsigned short temp_clientPort = ntohs(server_TCP_Pkt.clientPort);
         /* create/open Visitors.txt file */
         vistTxt = fopen("Visitors.txt", "a+");

         /* read from Travel.txt file */
         char str[TXT_FILE_BUFFER];
         char *token;
         int linenum = 1; // keeps track of line number when parsing
         unsigned short file_Step;
         char *client_name;

         while (fgets(str, TXT_FILE_BUFFER, vistTxt) != NULL)
         {
            token = strtok(str, ",");
            int tokIndex = 0; // index of the token

            while (token != NULL)
            {
               if (tokIndex == 0)
               {
                  file_Step = atoi(token);
               }
               if (tokIndex == 1 && atoi(token) == temp_clientPort) // searching for client port
               {                                                    // reads index with the client ID
                  found_client_flag = 1;
               }
               if (tokIndex == 2)
               {
                  client_name = token;
               }

               if (tokIndex == 2 && found_client_flag)
               { // reaches at the end of the line
                  break;
               }
               token = strtok(NULL, ",");
               tokIndex += 1;
            }

            if (found_client_flag)
            {
               break;
            }
            linenum += 1;
         }
         if (found_client_flag)
         {
            if (ntohs(server_TCP_Pkt.stepNum) == 2)
            {
               server_TCP_Pkt.serverScrtCode = htons(SERVER_SECRET_CODE);
            }
            else if (ntohs(server_TCP_Pkt.stepNum) == 3)
            {
               server_TCP_Pkt.serverScrtCode = htons(SERVER_SECRET_CODE);
            }
            else
            {
               server_TCP_Pkt.serverScrtCode = htons(0);
            }

            /*-------------------------------------------------------*/
            /* close Visitors.txt file*/
            fclose(vistTxt);

            /* reopen vistors.txt file */
            vistTxt = fopen("Visitors.txt", "r");

            /* open/create temp visitors.txt file */
            temp_vistTxt = fopen("tempVisitors.txt", "w+");

            int temp_lineNum = 1;
            while (fgets(str, TXT_FILE_BUFFER, vistTxt) != NULL)
            {
               if (temp_lineNum != linenum)
               {
                  fprintf(temp_vistTxt, "%s", str);
               }
               temp_lineNum += 1;
            }
            /* append new updated data from server */
            size_t length = sizeof(server_TCP_Pkt.message);
            if (server_TCP_Pkt.message[length - 1] == '\n')
            {
               fprintf(temp_vistTxt, "%d, %d, %s", ntohs(server_TCP_Pkt.stepNum), ntohs(server_TCP_Pkt.clientPort), server_TCP_Pkt.message);
            }
            else
            {
               fprintf(temp_vistTxt, "%d, %d, %s\n", ntohs(server_TCP_Pkt.stepNum), ntohs(server_TCP_Pkt.clientPort), server_TCP_Pkt.message);
            }
            fclose(temp_vistTxt);
            fclose(vistTxt);

            /* copy to orignal file */

            /* open Vistors.txt file in write mode */
            vistTxt = fopen("Visitors.txt", "w+");

            /* open tempVisitors.txt file in read mode */
            temp_vistTxt = fopen("tempVisitors.txt", "r");

            /* read from temp and copy to visitors.txt file */
            while (fgets(str, TXT_FILE_BUFFER, temp_vistTxt) != NULL)
            {
               fprintf(vistTxt, "%s", str);
            }

            /* close temp & Vistors.txt file*/
            fclose(vistTxt);
            fclose(temp_vistTxt);
         }
         else
         {
            /*put new data into Visitors.txt */
            size_t length = sizeof(server_TCP_Pkt.message);
            if (server_TCP_Pkt.message[length - 1] == '\n')
            {
               fprintf(vistTxt, "%d, %d,%s", ntohs(server_TCP_Pkt.stepNum), ntohs(server_TCP_Pkt.clientPort), server_TCP_Pkt.message);
            }
            else
            {
               fprintf(vistTxt, "%d, %d,%s\n", ntohs(server_TCP_Pkt.stepNum), ntohs(server_TCP_Pkt.clientPort), server_TCP_Pkt.message);
            }
            /* close Visitors.txt file*/
            fclose(vistTxt);
         }

         if (ntohs(server_TCP_Pkt.stepNum) == 3)
         {
            strcpy(server_TCP_Pkt.message, SERVER_TRAVEL_LOCATION);
         }

         bytes_sent = send(sock_connection, &server_TCP_Pkt, sizeof(struct clientServerPkt), 0);
      }

      /* reset the flag */
      found_client_flag = 0;

      /* close the socket */

      close(sock_connection);
   }
}
