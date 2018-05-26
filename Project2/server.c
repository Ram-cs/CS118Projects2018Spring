#include "support.h"

int main(int argc, char **argv) {
    
    // Ensure valid command line args
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    
    // Create the socket for UDP connection.
    int sockfd;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket did not open.");
        exit(1);
    }
    
    // Get the port number.
    int portno = atoi(argv[1]);
    
    // Set up the server IP
    struct sockaddr_in serveraddr;
    memset((char *) &serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)portno);
    
    // Bind the socket to the port.
    if (bind(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
        perror("Socket did not bind.");
        exit(1);
    }
    
    // Get a UDP packet (datagram), then echo it.
    struct sockaddr_in clientaddr;
    socklen_t clientlen = sizeof(clientaddr); // Byte size of client's address
    int BUFSIZE = 1024;
    char buf[BUFSIZE]; // Buffer for the message
    int n; // Buffer size for the message
    
    struct hostent *hostp; // information about the client host
    char *hostaddrp; // Dotted decimal host address string
    
    while (1) {
        
        // Get an UDP packet from the client.
        memset(buf, 0, BUFSIZE);
        n = recvfrom(sockfd, buf, BUFSIZE, 0, (struct sockaddr *) &clientaddr, &clientlen);
        if (n < 0) {
            perror("recvfrom did not work");
            exit(1);
        }
        
        // Figure out which client sent the UDP packet.
        hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
                              sizeof(clientaddr.sin_addr.s_addr), AF_INET);
        if (hostp == NULL) {
            perror("gethostbyaddr did not work");
            exit(1);
        }
        hostaddrp = inet_ntoa(clientaddr.sin_addr);
        if (hostaddrp == NULL) {
            perror("inet_nota did not work");
            exit(1);
        }
        
        printf("server received datagram from %s (%s)\n", hostp->h_name, hostaddrp);
        //printf("server received %d/%d bytes: %s\n", strlen(buf), n, buf);
        
        // Send the input back to the client.
        n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *) &clientaddr, clientlen);
        if (n < 0) {
            perror("sendto did not work");
            exit(1);
        }
    }
}
