/* A simple server in the internet domain using TCP
   The port number is passed as an argument
   This version runs forever, forking off a separate
   process for each connection
*/
#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>  /* signal name macros, and the kill() prototype */
#include <ctype.h>
#include <fcntl.h>

void error(char *msg)
{
  perror(msg);
  exit(1);
}

int main(int argc, char *argv[])
{
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;

  if (argc < 2) {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }

  sockfd = socket(AF_INET, SOCK_STREAM, 0);  // create socket
  if (sockfd < 0)
    error("ERROR opening socket");
  memset((char *) &serv_addr, 0, sizeof(serv_addr));  // reset memory

  // fill in address info
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");

  listen(sockfd, 5);  // 5 simultaneous connection at most
  //accept connections
  newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

  if (newsockfd < 0)
    error("ERROR on accept");

  int n;
  char buffer[1024];

  memset(buffer, 0, 1024);  // reset memory

  //read client's message
  n = read(newsockfd, buffer, 1023);
  if (n < 0) error("ERROR reading from socket");
  printf("Here is the message: %s\n", buffer);

  int sizeOfMessage = strlen(buffer);

  // Get the position of the "H" in "HTTP/"
  // This position will be used as the end position while parsing for the file name.
  int endPos = -1;
  for (int i = 0; i < sizeOfMessage; i++)
    {
      if (buffer[i] == 'H' && buffer[i+1] == 'T' && buffer[i+2] == 'T' &&
          buffer[i+3] == 'P' && buffer[i+4] == '/')
        {
          endPos = i;
          break;
        }
    }

  // Here, we parse through the HTTP request to find the file name we want to retreive
  int maxFileNameSize = 100;
  char fileName[maxFileNameSize];
  for (int i = 0; i < sizeOfMessage; i++)
    {
      // If we find '/', then the next character is the beginning of the file name.
      if (buffer[i] == '/')
        {
          int k = 0;
          // Here, we append the characters of the file name from the request message
          // to the fileName buffer.
          for (int j = i+1; j < endPos-1; j++, k++)
            {
              fileName[k] = buffer[j];
            }
          // We end the fileName buffer with '\0' to indicate the end of the fileName C-string.
          fileName[k] = '\0';
          break;
        }
    }

  // Fix "%20" to " " (The browser interprets a space as "%20")
   for (int i = 0; fileName[i] != '\0'; i++)
    {
      if (fileName[i] == '%' && fileName[i+1] == '2' && fileName[i+2] == '0')
        {
          fileName[i] = ' ';
          fileName[i+1] = ' ';
          fileName[i+2] = ' ';
        }
    }

  // Remove spaces from the file name string.
   for (int i = 0; fileName[i] != '\0'; )
    {
      if (fileName[i] == ' ')
        {
          for (int j = i; fileName[j] != '\0'; j++)
            {
              fileName[j] = fileName[j+1];
            }
        }
      else
        i++;
    }

  // Make all alphabet characters lowercase.
   for (int i = 0; fileName[i] != '\0'; i++)
    {
      if (isalpha(fileName[i]))
        {
          fileName[i] = tolower(fileName[i]);
        }
    }

  // Get the file extention type
  char fileType[10];
  for (int i = 0; fileName[i] != '\0'; i++)
    {
      if (fileName[i] == '.')
        {
          int k = 0;
          for (int j = i+1; fileName[j] != '\0'; j++, k++)
            {
              fileType[k] = fileName[j];
            }
          break;
        }
    }

  // TODO: Use strcmp to compare file types to html, txt, gif, etc.

  // Open the file so we can verify that it exists in our current directory.
  int fd = open(fileName, O_WRONLY | O_APPEND);
  if(fd < 0)
    printf("ERROR\n");
  else
    printf("FILE OPENED FINE\n");

  printf("%s\n", fileName);
  printf("%s\n", fileType);



  //reply to client

  n = write(newsockfd, "I got your message", 18);
  if (n < 0) error("ERROR writing to socket");
    close(newsockfd);  // close connection

  close(sockfd);

  return 0;
}
