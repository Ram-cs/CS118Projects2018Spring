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

    struct timeval read_timeout;
    read_timeout.tv_sec = 2;
    read_timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof read_timeout);
    
    // Get a UDP packet (datagram), then echo it.
    struct sockaddr_in clientaddr;
    socklen_t clientlen = sizeof(clientaddr); // Byte size of client's address
    int n; // Buffer size for the message
    
    // Wait for a SYN packet from the client.
    /*
    TCP_Packet * SYN_Packet = malloc(sizeof(TCP_Packet));
    //Received SYN
    n = recvfrom(sockfd, SYN_Packet, sizeof(*SYN_Packet), 0, (struct sockaddr *) &clientaddr, &clientlen);
    
    printf("Receiving packet %d\n", SYN_Packet->ackNum);

    
    // Create a SYNACK packet.
    TCP_Packet SYNACK_Packet;
    SYNACK_Packet.seqNum = 1;
    SYNACK_Packet.ackNum = 1;
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
    */
    // Wait for the final (request) packet in the 3-way handshake--> Established connection
    TCP_Packet * Request_Packet = malloc(sizeof(TCP_Packet));
    n = recvfrom(sockfd, Request_Packet, sizeof(*Request_Packet), 0, (struct sockaddr *) &clientaddr, &clientlen);
    if (n == -1) {
        error("reveive from error");
    }
    printf("Receiving Packet %d\n", Request_Packet->ackNum);
    
    //DIVIDE FILE INTO SMALL CHUNK, meaning read file upto the allowed size and repeat the process
    FILE* fd = NULL;
    char buffer[PAYLOAD_SIZE]; //1024
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

    int expected_ACK_STATE = 0;

    TCP_Packet packet;
    packet.seqNum = 0;
    packet.ackNum = 0;
    packet.SYN = 0;
    packet.FIN = 0;

    TCP_Packet * ackPacket = malloc(sizeof(TCP_Packet));

    
    if (fileSize <= PAYLOAD_SIZE) { // if the size of fd is less or equal to PAYLOAD_SIZE
      
      packet.PKG_TYPE = 1;
      fread(buffer, 1, fileSize, fd);
      //strcpy(packet.payload, buffer);
      packet.payload_size = fileSize;
      memcpy(packet.payload, buffer, sizeof(buffer));

      
      printf("Sending packet %d 5120\n", packet.seqNum);
      if (sendto(sockfd, (struct TCP_Packet *) &packet, 24 + fileSize, 0, (const struct sockaddr *) &clientaddr, clientlen)== -1) {
        error("sendto() error");
      }

      // Start the timer here

      fileSize = 0;
      //printf("Receiving Packet %d\n", ackPacket->ackNum);
      n = recvfrom(sockfd, ackPacket, sizeof(*ackPacket), 0, (struct sockaddr *) &clientaddr, &clientlen);
      if (n == -1)
	{
	  while (n == -1)
	    {
	      sendto(sockfd, (struct TCP_Packet *) &packet, 24 + fileSize, 0, (const struct sockaddr *) &clientaddr, clientlen);
	      printf("Sending packet %d 5120 Retransmission\n", packet.seqNum);

	      n = recvfrom(sockfd, ackPacket, sizeof(*ackPacket), 0, (struct sockaddr *) &clientaddr, &clientlen);
	      printf("Receiving Packet %d\n", ackPacket->ackNum);
	    }
	}
      else
	printf("Receiving Packet %d\n", ackPacket->ackNum);

      if (ackPacket->ackNum == expected_ACK_STATE)
      {
	if (expected_ACK_STATE == 0)
	  expected_ACK_STATE = 1;
	else if (expected_ACK_STATE == 1)
	  expected_ACK_STATE = 0;
        // stop the timer here
      }
      // else if (ackPacket->ackNum != expected_ACK_state), DO NOTHING
        

    } 
    else { // if the size is greater than the payload size

      packet.PKG_TYPE = 0;
      fread(buffer, 1, PAYLOAD_SIZE, fd);
      //strcpy(packet.payload, buffer);
      packet.payload_size = PAYLOAD_SIZE;
      memcpy(packet.payload, buffer, sizeof(buffer));

      printf("Sending packet %d 5120\n", packet.seqNum);
      if (sendto(sockfd, (struct TCP_Packet *) &packet, MAX_PKT_LENGTH, 0, (const struct sockaddr *) &clientaddr, clientlen)== -1) {
	error("sendto() error");
      }
      fileSize -= PAYLOAD_SIZE;

      n = recvfrom(sockfd, ackPacket, sizeof(*ackPacket), 0, (struct sockaddr *) &clientaddr, &clientlen);
      if (n == -1)
	{
	  while (n == -1)
	    {
	      sendto(sockfd, (struct TCP_Packet *) &packet, MAX_PKT_LENGTH, 0, (const struct sockaddr *) &clientaddr, clientlen);
	      printf("Sending packet %d 5120 Retransmission\n", packet.seqNum);

	      n = recvfrom(sockfd, ackPacket, sizeof(*ackPacket), 0, (struct sockaddr *) &clientaddr, &clientlen);
	      printf("Receiving Packet %d\n", ackPacket->ackNum);
	    }
	}
      else
	printf("Receiving Packet %d\n", ackPacket->ackNum);

      if (ackPacket->ackNum == expected_ACK_STATE)
      {
	if (expected_ACK_STATE == 0)
	  expected_ACK_STATE = 1;
	else if (expected_ACK_STATE == 1)
	  expected_ACK_STATE = 0;
        // stop the timer here
      } 
        
      while(fileSize > 0) {

        packet.seqNum = expected_ACK_STATE;
	packet.ackNum = expected_ACK_STATE;
	packet.SYN = 0;
	packet.FIN = 0;

        if (fileSize <= PAYLOAD_SIZE) { // if the size of fd is less or equal to PAYLOAD_SIZE

          packet.PKG_TYPE = 1;
	  fread(buffer, 1, fileSize, fd);
	  //strcpy(packet.payload, buffer);
	  packet.payload_size = fileSize;
	  memcpy(packet.payload, buffer, sizeof(buffer));

	  printf("Sending packet %d 5120\n", packet.seqNum);
	  if (sendto(sockfd, (struct TCP_Packet *) &packet, 24 + fileSize, 0, (const struct sockaddr *) &clientaddr, clientlen)== -1) {
	    error("sendto() error");
	  }
          
	  fileSize = 0;
	  //printf("Receiving Packet %d\n", ackPacket->ackNum);
          
          
	  n = recvfrom(sockfd, ackPacket, sizeof(*ackPacket), 0, (struct sockaddr *) &clientaddr, &clientlen);
	  if (n == -1)
	    {
	      while (n == -1)
		{
		  sendto(sockfd, (struct TCP_Packet *) &packet, 24 + fileSize, 0, (const struct sockaddr *) &clientaddr, clientlen);
		  printf("Sending packet %d 5120 Retransmission\n", packet.seqNum);

		  n = recvfrom(sockfd, ackPacket, sizeof(*ackPacket), 0, (struct sockaddr *) &clientaddr, &clientlen);
		  printf("Receiving Packet %d\n", ackPacket->ackNum);
		}
	    }
	  else
	    printf("Receiving Packet %d\n", ackPacket->ackNum);
	  
          if (ackPacket->ackNum == expected_ACK_STATE)
	    {
	      if (expected_ACK_STATE == 0)
		expected_ACK_STATE = 1;
	      else if (expected_ACK_STATE == 1)
		expected_ACK_STATE = 0;
	      // stop the timer here
	    }
          
	}

        else {

          packet.PKG_TYPE = 0;
	  fread(buffer, 1, PAYLOAD_SIZE, fd);
	  //strcpy(packet.payload, buffer);
	  packet.payload_size = PAYLOAD_SIZE;
	  memcpy(packet.payload, buffer, sizeof(buffer));

	  printf("Sending packet %d 5120\n", packet.seqNum);
	  if (sendto(sockfd, (struct TCP_Packet *) &packet, MAX_PKT_LENGTH, 0, (const struct sockaddr *) &clientaddr, clientlen)== -1) {
	    error("sendto() error");
	  }
	  fileSize -= PAYLOAD_SIZE;

	  //printf("Receiving Packet %d\n", ackPacket->ackNum);
	  n = recvfrom(sockfd, ackPacket, sizeof(*ackPacket), 0, (struct sockaddr *) &clientaddr, &clientlen);
	  if (n == -1)
	    {
	      while (n == -1)
		{
		  sendto(sockfd, (struct TCP_Packet *) &packet, MAX_PKT_LENGTH, 0, (const struct sockaddr *) &clientaddr, clientlen);
		  printf("Sending packet %d 5120 Retransmission\n", packet.seqNum);

		  n = recvfrom(sockfd, ackPacket, sizeof(*ackPacket), 0, (struct sockaddr *) &clientaddr, &clientlen);
		  printf("Receiving Packet %d\n", ackPacket->ackNum);
		}
	    }
	  else
	    printf("Receiving Packet %d\n", ackPacket->ackNum);

	  if (ackPacket->ackNum == expected_ACK_STATE)
	    {
	      if (expected_ACK_STATE == 0)
		expected_ACK_STATE = 1;
	      else if (expected_ACK_STATE == 1)
		expected_ACK_STATE = 0;
	      // stop the timer here
	    }

	  
        }

      }
    }
     
    fclose(fd);
    
    return 0;
}
