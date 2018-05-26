
#include "support.h"

int main(int argc, char **argv) {
    
    // Ensure valid command line args
    if (argc != 4) {
        fprintf(stderr,"usage: %s <hostname> <port> <filename>\n", argv[0]);
        exit(0);
    }
    
    // Get the hostname and the port number.
    // char *hostname = argv[1];
    int portno = atoi(argv[2]);
    char *filename = argv[3];
    
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

    
    /*
    int BUFSIZE = 1024;
    char buf[BUFSIZE];
    
    memset(buf, 0, BUFSIZE);
    printf("Please enter msg: ");
    fgets(buf, BUFSIZE, stdin);
    */    


    socklen_t serverlen = sizeof(serveraddr);
    int n;
  
    // Begin the TCP 3-way Handshake
    
    // First, make the SYN_Packet
    TCP_Packet SYN_Packet;
    SYN_Packet.seqNum = 1024;
    SYN_Packet.ackNum = 0;
    SYN_Packet.SYN = 1;
    SYN_Packet.FIN = 0;
    
    //n = sendto(sockfd, buf, strlen(buf), 0, (const struct sockaddr *) &serveraddr, serverlen);

    // Then, send it to the server.
    n = sendto(sockfd, (struct TCP_Packet *) &SYN_Packet, sizeof(SYN_Packet), 0, (const struct sockaddr *) &serveraddr, serverlen);
    if (n < 0) {
        perror("sendto did not work");
        exit(0);
    }

    // Wait for the server to reply with a SYNACK packet.
    TCP_Packet * SYNACK_Packet = malloc(sizeof(TCP_Packet));
    n = recvfrom(sockfd, SYNACK_Packet, sizeof(*SYNACK_Packet), 0, (struct sockaddr *) &serveraddr, &serverlen);

    printf("Received SYNACK with ack number: %d\n", SYNACK_Packet->ackNum);

    // Make the Packet sending requested filename
    TCP_Packet Request_Packet;
    Request_Packet.seqNum = SYNACK_Packet->ackNum + 1024;
    Request_Packet.ackNum = SYNACK_Packet->seqNum + 1024;
    Request_Packet.SYN = 0;
    Request_Packet.FIN = 0;
    Request_Packet.payload = filename;

    printf("The client sends a request with filename: %s\n", Request_Packet.payload);

    // Then send it to the server
    n = sendto(sockfd, (struct TCP_Packet *) &Request_Packet, sizeof(Request_Packet), 0, (const struct sockaddr *) &serveraddr, serverlen);
    if (n < 0) {
      perror("sendto did not work");
      exit(0);
    }


    
    // BELOW CODE NOT RELAVANT, BUT SAVED FOR FUTURE REFERENCE
    /*
    // Print the server's reply
    n = recvfrom(sockfd, buf, strlen(buf), 0, (struct sockaddr *) &serveraddr, &serverlen);
    if (n < 0) {
        perror("recvfrom did not work");
        exit(0);
    }
    printf("Echo from server: %s", buf);
    */
    return 0;
}
