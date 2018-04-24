
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

//DEFINE CONTENT TYPE FOR HTTP

#define TEXT "Content-Type: text/plain\r\n"
#define HTML "Content-Type: text/html\r\n"
#define JPG "Content-Type: image/jpeg\r\n"
#define JPEG "Content-Type: image/jpeg\r\n"
#define GIF "Content-Type: image/gif\r\n"
#define BIN "Content-Type: application/octet-stream\r\n"

//DEFINE STATUS

#define STATUS_OK "HTTP/1.1 200 OK\r\n"
#define HTTP_STATUS_NOT_FOUND "HTTP/1.1 404 Not Found\r\n"
#define FOUR_O_FOUR_NOT_FOUND "\r\n <html> <body> <h1> 404 Not Found </h1> </body> </html>"
#define DEFAULT_CONTENT_LENGTH "Content-Length: 342\r\n"

void error(char *msg)
{
    perror(msg);
    exit(1);
}


int flag = 1;

int i = 0;
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
    
    while (1) {
        if (listen(sockfd, 5) == 0) { // 5 simultaneous connection at most
            printf("Server Listening ...\n");
        } else {
            printf("[-] Error in binding\n");
        }
        //accept connections
        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        
        if (newsockfd < 0)
            error("ERROR on accept");
        
        long n;
        char buffer[1024];
        
        memset(buffer, 0, 1024);  // reset memory
        
        //read client's message
        n = read(newsockfd, buffer, 1023);
        if (n < 0) error("ERROR reading from socket");
        printf("%s\n", buffer);
        
        long sizeOfMessage = strlen(buffer);
        
        //++++++++++++++++++++++++++++++++++++
        //==========PARSING REQUEST==========+
        //++++++++++++++++++++++++++++++++++++
        
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
        //=============== TAKING CARE OF SPACE ======================
        // Fix "%20" to " " (The browser interprets a space as "%20")
        for (int i = 0; fileName[i] != '\0'; )
        {
            if (fileName[i] == '%' && fileName[i+1] == '2' && fileName[i+2] == '0')
            {
                fileName[i] = ' ';
                for (int j = i+1; fileName[j] != '\0'; j++)
                {
                    fileName[j] = fileName[j+1];
                }
                for (int j = i+1; fileName[j] != '\0'; j++)
                {
                    fileName[j] = fileName[j+1];
                }
            }
            else
                i++;
        }
        
        
        //============ MAKING ALPHABET LOWERCASE =====================
        for (int i = 0; fileName[i] != '\0'; i++)
        {
            if (isalpha(fileName[i]))
            {
                fileName[i] = tolower(fileName[i]);
            }
        }
        
        //============ GETTING FILE EXTeNSION ============
        char fileType[10] = "bin\0"; // binary is default.
        for (int i = 0; fileName[i] != '\0'; i++)
        {
            if (fileName[i] == '.')
            {
                int k = 0;
                for (int j = i+1; fileName[j] != '\0'; j++, k++)
                {
                    fileType[k] = fileName[j];
                }
                fileType[k] = '\0';
                break;
            }
        }
        
        
        //++++++++++++++++++++++++++++++++++++
        //==========HTTP RESPONSE============+
        //++++++++++++++++++++++++++++++++++++
        
        
        // Open the file so we can verify that it exists in our current directory.
        //FILE *fp = fopen(fileName,"r");
        int fd = open(fileName, O_RDONLY);
        //IF file can not open means we don't have file then send "404 file not found" error
        if(fd <= 0) {
            write(newsockfd, HTTP_STATUS_NOT_FOUND, 24);
            write(newsockfd, DEFAULT_CONTENT_LENGTH, 21);
            write(newsockfd, HTML, 25);
            write(newsockfd, FOUR_O_FOUR_NOT_FOUND,57);
            write(newsockfd, "\r\n", 2);
            close(newsockfd); //close client socket
            
        } else { //conforming requested file is in our directory
            
            printf("FILE OPENED FINE\n");
            char fileBuffer[100000000];
            FILE *fp = fopen(fileName,"r");
            
            fseek(fp, 0 , SEEK_END);
            long fileSize = ftell(fp);
            fseek(fp, 0 , SEEK_SET);
            
            char buffer [50];
            
            
            sprintf(buffer, "Content-Length: %ld", fileSize);
            
            int count = 0;
            long size = fileSize;
            
            while (size > 0) {
                size = size/ 10;
                count++;
            }
            
            //==========================HTML RESPONSE ===================
            n = write(newsockfd, STATUS_OK, 17);
            // Support file extensions: *.html, *.htm, and *.txt *.jpg, *.jpeg, and *.gif
            //find the right extension of the file, and write HTTP appropriatly
            
            n = write(newsockfd, buffer, 18 + count);
            write(newsockfd, "\r\n", 2);
            if (strcmp(fileType, "html") == 0) {
                //reply to client
                n = write(newsockfd, HTML, 25);
            } else if (strcmp(fileType, "htm") == 0) {
                n = write(newsockfd, HTML, 25);
            } else if (strcmp(fileType, "txt") == 0) {
                n = write(newsockfd, TEXT, 26);
            } else if (strcmp(fileType, "jpeg") == 0) {
                n = write(newsockfd, JPEG, 26);
            } else if (strcmp(fileType, "jpg") == 0) {
                n = write(newsockfd, JPG, 26);
            } else if (strcmp(fileType, "gif") == 0) {
                n = write(newsockfd, GIF, 25);
            } else if (strcmp(fileType, "bin") == 0) {
                n = write(newsockfd, BIN, 40);
            } else {
                n = write(newsockfd, FOUR_O_FOUR_NOT_FOUND, 13);
            }
            
            write(newsockfd, "\r\n", 2);
            if (n < 0) error("ERROR writing to socket"); //if we can not write file to client
            
            //Once we send the extension to client socket, send the real file now
            // Send the file back to the client browser.
            
            read(fd, fileBuffer, fileSize);
            write(newsockfd, fileBuffer, fileSize);
            
            close(newsockfd);  // close client socket after done with response
            
        }
        
    }
}

