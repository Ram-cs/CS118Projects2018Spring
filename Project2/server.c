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
        perror("Socket can not create.");
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
    int n; // Buffer size for the message
    
    // Wait for a SYN packet from the client.
    TCP_Packet * SYN_Packet = malloc(sizeof(TCP_Packet));
    //Received SYN
    n = recvfrom(sockfd, SYN_Packet, sizeof(*SYN_Packet), 0, (struct sockaddr *) &clientaddr, &clientlen);
    
    printf("Receiving packet %d\n", SYN_Packet->ackNum);
    
    // Create a SYNACK packet.
    TCP_Packet SYNACK_Packet;
    SYNACK_Packet.seqNum = 2000;
    SYNACK_Packet.ackNum = SYN_Packet->seqNum + HEADER_SIZE;
    SYNACK_Packet.SYN = 1;
    SYNACK_Packet.FIN = 0;
    SYNACK_Packet.PKG_TYPE = 0;
    
    // Send SYN and ACK.
    n = sendto(sockfd, (struct TCP_Packet *) &SYNACK_Packet, sizeof(SYNACK_Packet), 0, (struct sockaddr *) &clientaddr, clientlen);
    if (n < 0) {
        perror("sendto did not work");
        exit(1);
    }
    printf("Sending packet %d 5120 SYN\n", SYNACK_Packet.seqNum);
    
    // Wait for the final (request) packet in the 3-way handshake--> Established connection
    TCP_Packet * Request_Packet = malloc(sizeof(TCP_Packet));
    n = recvfrom(sockfd, Request_Packet, sizeof(*Request_Packet), 0, (struct sockaddr *) &clientaddr, &clientlen);
    if (n == -1) {
        error("reveive from error");
    }
    printf("Receiving Packet %d\n", Request_Packet->ackNum);
    
    //DIVIDE FILE INTO SMALL CHUNK, meaning read file upto the allowed size and repeat the process
    FILE* fd = NULL;
    char buffer[MAX_PKT_LENGTH]; //1024
    // size_t byte_read = 0;
    fd = fopen(Request_Packet->payload, "r"); //open the client file
    if(fd == NULL) { //if file is empty then exit
      error("File is empty");
    }

    fseek(fd, 0 , SEEK_END);
    long fileSize = ftell(fd);
    fseek(fd, 0 , SEEK_SET);

    //TCP_Packet packets[5];
    //int curr_packet = 0;

    TCP_Packet packet;
    packet.seqNum = Request_Packet->ackNum;
    packet.ackNum = Request_Packet->seqNum + MAX_PKT_LENGTH;
    packet.SYN = 0;
    packet.FIN = 0;

    TCP_Packet * ackPacket = malloc(sizeof(TCP_Packet));

    if (fileSize <= PAYLOAD_SIZE) { // if the size of fd is less or equal to PAYLOAD_SIZE
      
      packet.PKG_TYPE = 1;
      fread(buffer, 1, fileSize, fd);
      strcpy(packet.payload, buffer);
      
      printf("Sending packet %d 5120\n", packet.seqNum);
      if (sendto(sockfd, (struct TCP_Packet *) &packet, 20 + fileSize, 0, (const struct sockaddr *) &clientaddr, clientlen)== -1) {
        error("sendto() error");
      }
      fileSize = 0;
      printf("Receiving Packet %d\n", ackPacket->ackNum);
      if (recvfrom(sockfd, ackPacket, sizeof(*ackPacket), 0, (struct sockaddr *) &clientaddr, &clientlen) == -1) {
        error("reveive from error");
      }
    } 
    else {

      packet.PKG_TYPE = 0;
      fread(buffer, 1, PAYLOAD_SIZE, fd);
      strcpy(packet.payload, buffer);

      printf("Sending packet %d 5120\n", packet.seqNum);
      if (sendto(sockfd, (struct TCP_Packet *) &packet, MAX_PKT_LENGTH, 0, (const struct sockaddr *) &clientaddr, clientlen)== -1) {
	error("sendto() error");
      }
      fileSize -= PAYLOAD_SIZE;

      printf("Receiving Packet %d\n", ackPacket->ackNum);
      if (recvfrom(sockfd, ackPacket, sizeof(*ackPacket), 0, (struct sockaddr *) &clientaddr, &clientlen) == -1) {
	error("reveive from error");
      }
        
      while(fileSize > 0) {

        packet.seqNum = ackPacket->ackNum;
	packet.ackNum = ackPacket->seqNum + MAX_PKT_LENGTH;
	packet.SYN = 0;
	packet.FIN = 0;

        if (fileSize <= PAYLOAD_SIZE) { // if the size of fd is less or equal to PAYLOAD_SIZE

          packet.PKG_TYPE = 1;
	  fread(buffer, 1, fileSize, fd);
	  strcpy(packet.payload, buffer);

	  printf("Sending packet %d 5120\n", packet.seqNum);
	  if (sendto(sockfd, (struct TCP_Packet *) &packet, 20 + fileSize, 0, (const struct sockaddr *) &clientaddr, clientlen)== -1) {
	    error("sendto() error");
	  }
	  fileSize = 0;
	  printf("Receiving Packet %d\n", ackPacket->ackNum);
	  if (recvfrom(sockfd, ackPacket, sizeof(*ackPacket), 0, (struct sockaddr *) &clientaddr, &clientlen) == -1) {
	    error("reveive from error");
	  }
	}

        else {

          packet.PKG_TYPE = 0;
	  fread(buffer, 1, PAYLOAD_SIZE, fd);
	  strcpy(packet.payload, buffer);

	  printf("Sending packet %d 5120\n", packet.seqNum);
	  if (sendto(sockfd, (struct TCP_Packet *) &packet, MAX_PKT_LENGTH, 0, (const struct sockaddr *) &clientaddr, clientlen)== -1) {
	    error("sendto() error");
	  }
	  fileSize -= PAYLOAD_SIZE;

	  printf("Receiving Packet %d\n", ackPacket->ackNum);
	  if (recvfrom(sockfd, ackPacket, sizeof(*ackPacket), 0, (struct sockaddr *) &clientaddr, &clientlen) == -1) {
	    error("reveive from error");
	  }
        }


        /*
	fread(buffer, 1, PAYLOAD_SIZE, fd); //loop untill file is greater than PAYLOAD_SIZE
	
        packets[curr_packet].seqNum = packets[curr_packet - 1].ackNum;
	packets[curr_packet].ackNum = packets[curr_packet - 1].seqNum + MAX_PKT_LENGTH;
	packets[curr_packet].SYN = 0;
	packets[curr_packet].FIN = 0;
	packets[curr_packet].PKG_TYPE = 0;
	strcpy(packets[curr_packet].payload, buffer);

        printf("Sending packet %d 5120\n", packets[curr_packet].seqNum); //output sequence number
        
	if (sendto(sockfd, (struct TCP_Packet *) &packets[curr_packet], MAX_PKT_LENGTH, 0, (const struct sockaddr *) &clientaddr, clientlen)== -1) {
	  error("sendto() error");
	}
        curr_packet++;
        fileSize = fileSize - MAX_PKT_LENGTH;

	if (fileSize <= 0) { // if read all files then exit the loop
	  break;
	}

	if (fileSize <= HEADER_SIZE) { // if file is less than HEADER_SIZE then break
	  fread(buffer, 1, fileSize, fd);
          
          packets[curr_packet].seqNum = packets[curr_packet - 1].ackNum;
	  packets[curr_packet].ackNum = packets[curr_packet - 1].seqNum + MAX_PKT_LENGTH;
	  packets[curr_packet].SYN = 0;
	  packets[curr_packet].FIN = 0;
	  packets[curr_packet].PKG_TYPE = 0;
	  strcpy(packets[curr_packet].payload, buffer);

          printf("Sending packet %d 5120\n", packets[curr_packet].seqNum);

	  if (sendto(sockfd, (struct TCP_Packet *) &packets[curr_packet], 20 + fileSize, 0, (const struct sockaddr *) &clientaddr, clientlen)== -1) {
	    error("sendto() error");
	  }
	  break;
	}
        */

      }
    }
     
    fclose(fd);
    
    //RECEIVED PACKAGE TO CLIENT
    
    /*
    for (int i = 0; i < NUM_PKG; i++) {
        if (recvfrom(sockfd, Request_Packet->payload, MAX_PKT_LENGTH, 0, (struct sockaddr *) &clientaddr, &clientlen)==-1) {
            error("reveive from error");
        }
        
        printf("Received packet %d\n", Request_Packet->seqNum); //output sequence number
        
        printf("Received packet from %s:%d\nData: %s\n\n",
               inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port), Request_Packet->payload);
    }
    */
    
    
    
    /*
     int BUFSIZE = 1024;
     char buf[BUFSIZE]; // Buffer for the message
     
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
     */
    return 0;
}
