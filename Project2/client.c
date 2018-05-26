
#include "support.h"

int main(int argc, char **argv) {
    
    // Ensure valid command line args
    if (argc != 3) {
        fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
        exit(0);
    }
    
    // Get the hostname and the port number.
    char *hostname = argv[1];
    int portno = atoi(argv[2]);
    
    // Create the socket for UDP connection
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket did not open.");
        exit(0);
    }
    
    // Find the server's DNS entry.
    /*
     struct hostent *server;
     server = gethostbyname(hostname);
     if (server == NULL) {
     fprintf(stderr,"ERROR, no such host as %s\n", hostname);
     exit(0);
     }
     */
    
    // Set up the server IP
    struct sockaddr_in serveraddr;
    memset((char *) &serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(portno);
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    //bcopy((char *)server->h_addr,
    //    (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    
    // Wait for a message from the client.
    int BUFSIZE = 1024;
    char buf[BUFSIZE];
    
    memset(buf, 0, BUFSIZE);
    printf("Please enter msg: ");
    fgets(buf, BUFSIZE, stdin);
    
    // Send a message to the server
    socklen_t serverlen = sizeof(serveraddr);
    int n;
    n = sendto(sockfd, buf, strlen(buf), 0, (const struct sockaddr *) &serveraddr, serverlen);
    if (n < 0) {
        perror("sendto did not work");
        exit(0);
    }
    
    // Print the server's reply
    n = recvfrom(sockfd, buf, strlen(buf), 0, (struct sockaddr *) &serveraddr, &serverlen);
    if (n < 0) {
        perror("recvfrom did not work");
        exit(0);
    }
    printf("Echo from server: %s", buf);
    return 0;
}
