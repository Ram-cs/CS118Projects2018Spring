
#include "support.h"

int main(int argc, char **argv) {
    
    // Ensure valid command line args
    if (argc != 4) {
        fprintf(stderr,"usage: %s <hostname> <port> <filename>\n", argv[0]);
        exit(1);
    }
    
    // Get the hostname and the port number.
    // char *hostname = argv[1];
    int portno = atoi(argv[2]);
    //char *filename = argv[3];
    
    // Create the socket for UDP connection
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket did not open.");
        exit(1);
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
    SYN_Packet.seqNum = 1000;
    SYN_Packet.ackNum = 1;
    SYN_Packet.SYN = 1;
    SYN_Packet.FIN = 0;
    SYN_Packet.PKG_TYPE = 0;
    
    //n = sendto(sockfd, buf, strlen(buf), 0, (const struct sockaddr *) &serveraddr, serverlen);
    
    // Then, send it to the server.
    n = sendto(sockfd, (struct TCP_Packet *) &SYN_Packet, sizeof(SYN_Packet), 0, (const struct sockaddr *) &serveraddr, serverlen);
    if (n < 0) {
        perror("sendto did not work");
        exit(1);
    }
    printf("Sending packet %d SYN\n", SYN_Packet.seqNum);
    
    // Wait for the server to reply with a SYNACK packet.
    TCP_Packet * SYNACK_Packet = malloc(sizeof(TCP_Packet));
    n = recvfrom(sockfd, SYNACK_Packet, sizeof(*SYNACK_Packet), 0, (struct sockaddr *) &serveraddr, &serverlen);
    printf("Receiving packet %d\n", SYNACK_Packet->ackNum);
    
    // Make the Packet sending requested filename
    TCP_Packet Request_Packet;
    Request_Packet.seqNum = SYNACK_Packet->ackNum;
    Request_Packet.ackNum = 1;
    Request_Packet.SYN = 0;
    Request_Packet.FIN = 0;
    Request_Packet.PKG_TYPE= 0;
    strcpy(Request_Packet.payload, argv[3]);
    
    // Then send it to the server
    n = sendto(sockfd, (struct TCP_Packet *) &Request_Packet, sizeof(Request_Packet), 0, (const struct sockaddr *) &serveraddr, serverlen);
    if (n < 0) {
        perror("sendto did not work");
        exit(1);
    }
    printf("Sending packet %d\n", Request_Packet.seqNum);

    FILE *fp;
    fp = fopen("received.data","w");

    int finish_file = 0;
    while(finish_file == 0)
    {

      TCP_Packet * Result_Packet0 = malloc(sizeof(TCP_Packet));
      n = recvfrom(sockfd, Result_Packet0, sizeof(*Result_Packet0), 0, (struct sockaddr *) &serveraddr, &serverlen);
      printf("Receiving packet %d\n", Result_Packet0->ackNum);

      // If the incoming packet is not a duplicate:
      if (Result_Packet0->ackNum != 1)
      { 
        fprintf(fp, "%s", Result_Packet0->payload);
      
        if (Result_Packet0->PKG_TYPE == 1)
          finish_file = 1;

        TCP_Packet ackPacket0;
        ackPacket0.seqNum = 0;
        ackPacket0.ackNum = 0;
        ackPacket0.SYN = 0;
        ackPacket0.FIN = 0;
        ackPacket0.PKG_TYPE= 0;

        n = sendto(sockfd, (struct TCP_Packet *) &ackPacket0, sizeof(ackPacket0), 0, (const struct sockaddr *) &serveraddr, serverlen);
        if (n < 0) {
          perror("sendto did not work");
          exit(1);
        }
        
        printf("Sending packet %d\n", ackPacket0.seqNum);
      }
      // Else it is a duplicate:
      else
      {
	TCP_Packet ackPacket1;
        ackPacket1.seqNum = 1;
        ackPacket1.ackNum = 1;
        ackPacket1.SYN = 0;
        ackPacket1.FIN = 0;
        ackPacket1.PKG_TYPE= 0;

        n = sendto(sockfd, (struct TCP_Packet *) &ackPacket1, sizeof(ackPacket1), 0, (const struct sockaddr *) &serveraddr, serverlen);
        if (n < 0) {
          perror("sendto did not work");
          exit(1);
        }
	
        printf("Sending packet %d\n", ackPacket1.seqNum);
      }
      
      if (finish_file == 1)
        break;

      TCP_Packet * Result_Packet1 = malloc(sizeof(TCP_Packet));
      n = recvfrom(sockfd, Result_Packet1, sizeof(*Result_Packet1), 0, (struct sockaddr *) &serveraddr, &serverlen);
      printf("Receiving packet %d\n", Result_Packet1->ackNum);
      
      // If the incoming packet is not a duplicate: 
      if (Result_Packet1->ackNum != 0)
      {
        fprintf(fp, "%s", Result_Packet1->payload);

        if (Result_Packet1->PKG_TYPE == 1)
          finish_file = 1;

        TCP_Packet ackPacket1;
        ackPacket1.seqNum = 1;
        ackPacket1.ackNum = 1;
        ackPacket1.SYN = 0;
        ackPacket1.FIN = 0;
        ackPacket1.PKG_TYPE= 0;

        n = sendto(sockfd, (struct TCP_Packet *) &ackPacket1, sizeof(ackPacket1), 0, (const struct sockaddr *) &serveraddr, serverlen);
        if (n < 0) {
          perror("sendto did not work");
          exit(1);
        }
        printf("Sending packet %d\n", ackPacket1.seqNum);
      }
      // Else it is a duplicate:
      else
      {
        TCP_Packet ackPacket0;
        ackPacket0.seqNum = 0;
        ackPacket0.ackNum = 0;
        ackPacket0.SYN = 0;
        ackPacket0.FIN = 0;
        ackPacket0.PKG_TYPE= 0;

        n = sendto(sockfd, (struct TCP_Packet *) &ackPacket0, sizeof(ackPacket0), 0, (const struct sockaddr *) &serveraddr, serverlen);
        if (n < 0) {
          perror("sendto did not work");
          exit(1);
        }
        printf("Sending packet %d\n", ackPacket0.seqNum);
      }

    }

    fclose(fp);
    return 0;
}
