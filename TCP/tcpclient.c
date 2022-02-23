/* tcp_ client.c */
/* Programmed by Adarsh Sethi */
/* Sept. 19, 2021 */

#include <stdio.h>      /* for standard I/O functions */
#include <stdlib.h>     /* for exit */
#include <string.h>     /* for memset, memcpy, and strlen */
#include <netdb.h>      /* for struct hostent and gethostbyname */
#include <sys/socket.h> /* for socket, connect, send, and recv */
#include <netinet/in.h> /* for sockaddr_in */
#include <unistd.h>     /* for close */

#define STRING_SIZE 1024

#define CLIENT_TCP_PORT 45522
#define CLIENT_VISITOR_NAME "Hayes-Jiang\n\0"

#define TXT_FILE_BUFFER 35

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

   int sock_client; /* Socket used by client */

   struct sockaddr_in server_addr;    /* Internet address structure that
                                        stores server address */
   struct hostent *server_hp;         /* Structure to store server's IP
                                        address */
   struct sockaddr_in client_addr;    /* Internet address structure that
                                        stores client address */
   unsigned short client_port;        /* Port number used by client (local port) */
   char server_hostname[STRING_SIZE]; /* Server's hostname */
   unsigned short server_port;        /* Port number used by server (remote port) */

   char sentence[STRING_SIZE];         /* send message */
   char modifiedSentence[STRING_SIZE]; /* receive message */
   unsigned int msg_len;               /* length of message */
   int bytes_sent, bytes_recd;         /* number of bytes sent or received */

   short gbl_Stepnum = 1; // global step number (Might not need this)
   unsigned short glb_server_secret = 0;
   char gbl_message[STRING_SIZE];
   int transmitt_flag = 1; // flag for transmitting to the server (stops send when stepnum ==3)
   int server_found = 0;
   strcpy(gbl_message, "*\n\0");
   FILE *trvlTxt;      /* pointer to travel.txt */
   FILE *temp_trvlTxt; /* pointer to temp travel.txt */
   for (unsigned short i = 48000; i <= 48999; i++)
   {
      server_found = 0;
      server_port = i; /* assign the server port to connect to */

      /* open a socket */

      if ((sock_client = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
      {
         perror("Client: can't open stream socket");
         exit(1);
      }
      client_port = CLIENT_TCP_PORT; /* This allows choice of any available local port */

      /* clear client address structure and initialize with client address */
      memset(&client_addr, 0, sizeof(client_addr));
      client_addr.sin_family = AF_INET;
      client_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* This allows choice of
                                           any host interface, if more than one
                                           are present */
      client_addr.sin_port = htons(client_port);

      /* bind the socket to the local client port */

      if (bind(sock_client, (struct sockaddr *)&client_addr,
               sizeof(client_addr)) < 0)
      {
         //         perror("Client: can't bind to local address\n");
         close(sock_client);
         exit(1);
      }

      /* Note: there is no need to initialize local client address information
               unless you want to specify a specific local port
               (in which case, do it the same way as in udpclient.c).
               The local address initialization and binding is done automatically
               when the connect function is called later, if the socket has not
               already been bound. */

      /* initialize server address information */

      strcpy(server_hostname, "localhost");
      if ((server_hp = gethostbyname(server_hostname)) == NULL)
      {
         perror("Client: invalid server hostname");
         close(sock_client);
         exit(1);
      }

      /* Clear server address structure and initialize with server address */
      memset(&server_addr, 0, sizeof(server_addr));
      server_addr.sin_family = AF_INET;
      memcpy((char *)&server_addr.sin_addr, server_hp->h_addr,
             server_hp->h_length);
      server_addr.sin_port = htons(server_port);

      /* connect to the server */

      if (connect(sock_client, (struct sockaddr *)&server_addr,
                  sizeof(server_addr)) < 0)
      {
         // perror("Client: can't connect to server");
         close(sock_client);
         //  exit(1);
      }
      else
      {
         server_found = 1;
      }
      /* create/open Travel.txt file */
      trvlTxt = fopen("Travel.txt", "a+");

      /* read from Travel.txt file */
      char str[TXT_FILE_BUFFER];
      char *token;
      int linenum = 1; // keeps track of line number when parsing
      unsigned short temp_Step;
      unsigned short server_scrt_code;
      int found_server_flag = 0; // flag for finding client
      char *server_loc;
      while (fgets(str, TXT_FILE_BUFFER, trvlTxt) != NULL)
      {
         token = strtok(str, ",");
         int tokIndex = 0; // index of the token

         while (token != NULL)
         {
            if (tokIndex == 0)
            {
               temp_Step = atoi(token);
            }
            if (tokIndex == 1 && atoi(token) == server_port) // searching for client port
            {                                                // reads index with the client ID
               found_server_flag = 1;
            }
            if (tokIndex == 2)
            {
               server_scrt_code = atoi(token);
            }
            if (tokIndex == 3)
            {
               server_loc = token;
            }

            if (tokIndex == 3 && found_server_flag)
            { // reaches at the end of the line
               break;
            }
            token = strtok(NULL, ",");
            tokIndex += 1;
         }
         if (found_server_flag)
         {
            break;
         }
         linenum += 1;
      }
      if (found_server_flag)
      {
         glb_server_secret = server_scrt_code;
         if (temp_Step != 3)
         {
            gbl_Stepnum = temp_Step + 1;
            if (gbl_Stepnum == 3)
            {
               strcpy(gbl_message, CLIENT_VISITOR_NAME);
            }
            else
            {
               strcpy(gbl_message, "*\n\0");
            }
         }
         else
         {
            transmitt_flag = 0; /* doesn't have to send transmit to the server anymore */
         }
      }

      if (transmitt_flag && server_found)
      {
         /* setting timeout */
         struct timeval timeout;
         timeout.tv_sec = 1;
         timeout.tv_usec = 0;
         setsockopt(sock_client, SOL_SOCKET, SO_RCVTIMEO, (const void *)&timeout, sizeof(timeout));

         /* initialize struct */
         struct clientServerPkt client_TCP_Pkt = {htons(gbl_Stepnum), htons(CLIENT_TCP_PORT), htons(server_port), htons(glb_server_secret), "*\n"};
         strcpy(client_TCP_Pkt.message, gbl_message);

         /* send message to server*/
         printf("Message sent: %d, %d, %d, %s\n", ntohs(client_TCP_Pkt.stepNum), ntohs(client_TCP_Pkt.serverPort), ntohs(client_TCP_Pkt.serverScrtCode), client_TCP_Pkt.message);
         bytes_sent = send(sock_client, &client_TCP_Pkt, sizeof(struct clientServerPkt), 0);

         /* get response from server */

         bytes_recd = recv(sock_client, &client_TCP_Pkt, sizeof(struct clientServerPkt), 0);

         if (bytes_recd <= 0)
         {
            // /* close the socket */

            // close(sock_client);
         }
         else
         {
            printf("Message recieved: %d, %d, %d, %s\n", ntohs(client_TCP_Pkt.stepNum), ntohs(client_TCP_Pkt.serverPort), ntohs(client_TCP_Pkt.serverScrtCode), client_TCP_Pkt.message);

            if (!found_server_flag)
            { // Case for if the server is not found in the Travel.txt file
               /*put new data into Travel.txt */
               size_t length = strlen(client_TCP_Pkt.message);
               if (client_TCP_Pkt.message[length - 1] == '\n')
               {
                  fprintf(trvlTxt, "%d, %d, %d, %s", ntohs(client_TCP_Pkt.stepNum), ntohs(client_TCP_Pkt.serverPort), ntohs(client_TCP_Pkt.serverScrtCode), client_TCP_Pkt.message);
               }
               else
               {
                  fprintf(trvlTxt, "%d, %d, %d, %s\n", ntohs(client_TCP_Pkt.stepNum), ntohs(client_TCP_Pkt.serverPort), ntohs(client_TCP_Pkt.serverScrtCode), client_TCP_Pkt.message);
               }
               /* close Travel.txt file*/
               fclose(trvlTxt);
            }
            else
            {
               //* case for making seperate file to copy over *//

               /* close Travel.txt file*/
               fclose(trvlTxt);

               /* reopen travel.txt file */
               trvlTxt = fopen("Travel.txt", "r");
               /* open/create temp travel.txt file */
               temp_trvlTxt = fopen("tempTravel.txt", "w+");
               int temp_lineNum = 1;

               while (fgets(str, TXT_FILE_BUFFER, trvlTxt) != NULL)
               {
                  if (temp_lineNum != linenum)
                  {
                     fprintf(temp_trvlTxt, "%s", str);
                  }
                  temp_lineNum += 1;
               }
               /* append new updated data from server */
               size_t length = strlen(client_TCP_Pkt.message);
               if (client_TCP_Pkt.message[length - 1] == '\n')
               {
                  fprintf(temp_trvlTxt, "%d, %d, %d, %s", ntohs(client_TCP_Pkt.stepNum), ntohs(client_TCP_Pkt.serverPort), ntohs(client_TCP_Pkt.serverScrtCode), client_TCP_Pkt.message);
               }
               else
               {
                  fprintf(temp_trvlTxt, "%d, %d, %d, %s\n", ntohs(client_TCP_Pkt.stepNum), ntohs(client_TCP_Pkt.serverPort), ntohs(client_TCP_Pkt.serverScrtCode), client_TCP_Pkt.message);
               }
               fclose(temp_trvlTxt);
               fclose(trvlTxt);

               /* copy to orignal file */

               /* open travel.txt file in write mode */
               trvlTxt = fopen("Travel.txt", "w+");

               /* open tempTravel.txt file in read mode */
               temp_trvlTxt = fopen("tempTravel.txt", "r");

               /* read from temp and copy to travel.txt file */
               while (fgets(str, TXT_FILE_BUFFER, temp_trvlTxt) != NULL)
               {
                  fprintf(trvlTxt, "%s", str);
               }

               /* close temp & Travel.txt file*/
               fclose(trvlTxt);
               fclose(temp_trvlTxt);
            }

            /* close the socket */

            close(sock_client);
         }
         /* close the socket */

         close(sock_client);
      }
   }
}
