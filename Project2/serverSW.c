#include "support.h"


int retransmitFlag = 0;

void retransmit(int sig)
{
  retransmitFlag = 1;
  signal(SIGALRM, retransmit);
}

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

    signal(SIGALRM, retransmit);

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
      strcpy(packet.payload, buffer);
      
      printf("Sending packet %d 5120\n", packet.seqNum);
      if (sendto(sockfd, (struct TCP_Packet *) &packet, 20 + fileSize, 0, (const struct sockaddr *) &clientaddr, clientlen)== -1) {
        error("sendto() error");
      }

      // Start the timer here

      fileSize = 0;
      printf("Receiving Packet %d\n", ackPacket->ackNum);
      if (recvfrom(sockfd, ackPacket, sizeof(*ackPacket), 0, (struct sockaddr *) &clientaddr, &clientlen) == -1) {
        error("reveive from error");
      }

      if (retransmitFlag == 1)
      {
        while (retransmitFlag == 1)
	{
          
          

          

        }
      }

      if (ackPacket->ackNum == expected_ACK_STATE)
      {
        expected_ACK_STATE = ~expected_ACK_STATE;
        // stop the timer here
      }
      // else if (ackPacket->ackNum != expected_ACK_state), DO NOTHING
        

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

      // Start the timer here

      printf("Receiving Packet %d\n", ackPacket->ackNum);
      if (recvfrom(sockfd, ackPacket, sizeof(*ackPacket), 0, (struct sockaddr *) &clientaddr, &clientlen) == -1) {
	error("reveive from error");
      }

      if (ackPacket->ackNum == expected_ACK_STATE)
      {
        expected_ACK_STATE = ~expected_ACK_STATE;
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
	  strcpy(packet.payload, buffer);

	  printf("Sending packet %d 5120\n", packet.seqNum);
	  if (sendto(sockfd, (struct TCP_Packet *) &packet, 20 + fileSize, 0, (const struct sockaddr *) &clientaddr, clientlen)== -1) {
	    error("sendto() error");
	  }
          // Set an alarm for 500 ms.
          ualarm(500000, 0);
          
	  fileSize = 0;
	  printf("Receiving Packet %d\n", ackPacket->ackNum);
          
          
	  if (recvfrom(sockfd, ackPacket, sizeof(*ackPacket), 0, (struct sockaddr *) &clientaddr, &clientlen) == -1) {
	    error("reveive from error");
	  }
          // Disable the previously set alarm.

          if (retransmitFlag == 1)
	    {
	      while (retransmitFlag == 1)
		{
                  printf("Sending packet %d 5120 Retransmission\n", packet.seqNum);
		  if (sendto(sockfd, (struct TCP_Packet *) &packet, 20 + fileSize, 0, (const struct sockaddr *) &clientaddr, clientlen)== -1) {
		    error("sendto() error");
		  }

                  ualarm(500000, 0);
                  fileSize = 0;
		  printf("Receiving Packet %d\n", ackPacket->ackNum);


		  if (recvfrom(sockfd, ackPacket, sizeof(*ackPacket), 0, (struct sockaddr *) &clientaddr, &clientlen) == -1) {
		    error("reveive from error");
		  }

                  if (ackPacket->ackNum == expected_ACK_STATE)
		    {
                      retransmitFlag = 0;
                      break;
		    }

		}
	    }

          if (ackPacket->ackNum == expected_ACK_STATE)
	    {
	      expected_ACK_STATE = ~expected_ACK_STATE;
	      // stop the timer here
              ualarm(0, 0);
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

      }
    }
     
    fclose(fd);
    
    return 0;
}
