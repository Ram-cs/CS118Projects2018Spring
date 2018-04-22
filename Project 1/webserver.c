
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
//
//DEFINE STATUS
#define STATUS_OK "HTTP/1.1 200 OK\r\n"
#define HTTP_STATUS_NOT_FOUND "HTTP/1.1 404 Not Found\r\n"
#define FOUR_O_FOUR_NOT_FOUND "404 Not Found"
#define DEFAULT_CONTENT_LENGTH "Content-Length: 342\r\n"

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
    
    if(listen(sockfd, 5) == 0) { // 5 simultaneous connection at most
        printf("Server Listening ...\n");
    } else {
        printf("[-] Error in binding\n");
    }
    
    while (1) {
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
        printf("Received Message: %s\n", buffer);
        
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
        
        
        //++++++++++++++++++++++++++++++++++++
        //==========HTTP RESPONSE============+
        //++++++++++++++++++++++++++++++++++++
        
        
        // Open the file so we can verify that it exists in our current directory.
        FILE *fp = fopen(fileName,"r");
        //IF file can not open means we don't have file then send "404 file not found" error
        if(fp <= 0) {
            write(newsockfd, HTTP_STATUS_NOT_FOUND, 30);
            write(newsockfd, DEFAULT_CONTENT_LENGTH, 30);
            write(newsockfd, TEXT, 30);
            write(newsockfd, FOUR_O_FOUR_NOT_FOUND, 30);
            close(newsockfd); //close client socket
            
        } else { //conforming requested file is in our directory
            printf("FILE OPENED FINE\n");
            //==========================HTML RESPONSE ===================
            // Support file extensions: *.html, *.htm, and *.txt *.jpg, *.jpeg, and *.gif
            //find the right extension of the file, and write HTTP appropriatly
            if (strcmp(fileType, "html") == 0) {
                //reply to client
                n = write(newsockfd, HTML, 30);
            } else if (strcmp(fileType, "htm") == 0) {
                n = write(newsockfd, HTML, 30);
            } else if (strcmp(fileType, "jpeg") == 0) {
                n = write(newsockfd, JPEG, 30);
            } else if (strcmp(fileType, "jpg") == 0) {
                n = write(newsockfd, JPG, 30);
            } else if (strcmp(fileType, "gif") == 0) {
                n = write(newsockfd, GIF, 30);
            } else {
                n = write(newsockfd, FOUR_O_FOUR_NOT_FOUND, 30);
            }
            
            if (n < 0) error("ERROR writing to socket"); //if we can not write file to client
            
            //Once we send the extension to client socket, send the real file now
            //to send the real file we need to find the size of file https://stackoverflow.com/questions/35390912/proper-way-to-get-file-size-in-c
            fseek(fp, 0 , SEEK_END);
            long fileSize = ftell(fp);
            fseek(fp, 0 , SEEK_SET);// needed for next read from beginning of file
            
            
            
            //sending the real file using "fwrite" or "send" or.... method
            //https://stackoverflow.com/questions/11952898/c-send-and-receive-file
            
            
            
            
            close(newsockfd);  // close client socket after done with response
            
        }
        
    }
    
}





