#include "support.h"
#include "support_RETRA.c"

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
    
    // Create a vector (size 5) of packets representing the cwnd and fill it up:
    vector cwnd;
    vector_init(&cwnd);
    long tempFileSize = fileSize;
    int expectedSeqNum = 0; // change the value of this after completing 3-way handshake
    while (cwnd.total < 5 && tempFileSize > 0)
    {
        TCP_Packet * packet = malloc(sizeof(TCP_Packet));
        packet->seqNum = expectedSeqNum;
        packet->ackNum = 0; // ackPacket->seqNum + MAX_PKT_LENGTH;
        packet->SYN = 0;
        packet->FIN = 0;
        // add more packet values here
        if (tempFileSize <= PAYLOAD_SIZE)
        {
            packet->PKG_TYPE = 1;
            fread(buffer, 1, tempFileSize, fd);
            packet->payload_size = tempFileSize;
            tempFileSize = 0;
        }
        else
        {
            packet->PKG_TYPE = 0;
            fread(buffer, 1, PAYLOAD_SIZE, fd);
            packet->payload_size = PAYLOAD_SIZE;
            tempFileSize -= PAYLOAD_SIZE;
        }
        memcpy(packet->payload, buffer, sizeof(buffer));
        
        vector_add(&cwnd, packet);
        expectedSeqNum += 1024;
    }
    
    /*
     int currOldestUnACKedPacket;
     time_t start_t, end_t;
     double diff_t;
     */
    
    vector timing;
    vector_init(&timing);
    int vector_index = 0;
    
    // Then, send data packets to the client
    for (int i = 0; i < cwnd.total; i++)
    {
        // Here, consider checking each packet's header to see if it has already been received. If so, don't send.
        TCP_Packet * pac = malloc(sizeof(TCP_Packet));
        pac = vector_get(&cwnd, i);
        
        //if (sendto(sockfd, (struct TCP_Packet *) &pac, MAX_PKT_LENGTH, 0, (const struct sockaddr *) &clientaddr, clientlen) != -1)
        if (sendto(sockfd, pac, sizeof(*pac), 0, (struct sockaddr *) &clientaddr, clientlen) != -1)
        {
            printf("Sending packet %d 5120\n", pac->seqNum);
            
            long long time;
            time =  sys_timestamp();
            
            vector_add(&timing, &time);
            vector_add(&timing, &(pac->seqNum)); //FIRST TIME, THEN SEQNUM
            vector_add(&timing, &(pac->ackNum)); //store the acknum
            
            /*
             if (i == 0)
             {
             // start timer
             time(&start_t);
             currOldestUnACKedPacket = pac->seqNum;
             }
             */
        }
    }
    
    struct pollfd thePoll[1];
    thePoll[0].fd = sockfd;
    thePoll[0].events = POLLIN;
    
    TCP_Packet * resent_packate = malloc(sizeof(TCP_Packet));
    long long *current_time = malloc(sizeof(100));
    long long *packate_time = malloc(sizeof(100));
    long long *vector_sequence = malloc(sizeof(100));
    long long *vector_ackNum = malloc(sizeof(100));
    
    while (cwnd.total > 0) // the cwnd contains a packet to send
    {
        
        // First, use poll to read from the socket to see if there is any ACK packets we need to inspect
        //Retrasmission of the packet that are lost
        
        
        for(int j = 0; j < timing.total; j = j + 3) {
            packate_time = vector_get(&timing, j);
            *current_time = sys_timestamp(); //get the current time of the system
            long long packate_timestamp = *current_time - *packate_time;
    
            if(packate_timestamp > 500) { //if packate time is greater than 500ms then resend the package
                int sequence  = j + 1;
                vector_sequence = vector_get(&timing, sequence); //sequence number
                int ack_num = sequence + 1;
                vector_ackNum = vector_get(&timing, ack_num); //ack num
                resent_packate->seqNum = *vector_sequence;
                resent_packate->SYN = 0;
                resent_packate->FIN = 0;
                resent_packate->PKG_TYPE = 0;
                resent_packate->ackNum = *vector_ackNum;
                
                if (sendto(sockfd, resent_packate, sizeof(*resent_packate), 0, (const struct sockaddr *) &clientaddr, clientlen) != -1)
                {
                    printf("Sending packet %d 5120\n", resent_packate->seqNum);
                }
                
                //then now start new time
                vector_delete(&timing, j); //delete time
                vector_delete((&timing), j); //delete sequence number
                vector_delete(&timing, j); //delete ack number
                
                *current_time = sys_timestamp(); //get current time
                //now add sequence number and time
                vector_add(&timing, current_time); //add current time
                vector_add(&timing, vector_sequence); //add sequence number
                vector_add(&timing, vector_ackNum); //add ack number
            }
            
        }
        
        int r = poll(thePoll, 1, 500);
        if (r > 0)
        {
            // If there is data to be read at the socket (the client has responded with an ACK)
            if (thePoll[0].revents & POLLIN)
            {
                TCP_Packet * ackPacket = malloc(sizeof(TCP_Packet));
                n = recvfrom(sockfd, ackPacket, sizeof(*ackPacket), 0, (struct sockaddr *) &clientaddr, &clientlen);
                
                /*
                 time(&end_t);
                 diff_t = difftime(end_t, start_t);
                 if ()
                 */
                
                // If we have indeed received a packet...
                if (n != 1)
                {
                    printf("Receiving Packet %d\n", ackPacket->ackNum);
                    // For every packet currently in the window...
                    for (int i = 0; i < cwnd.total; i++)
                    {
                      
                        
                        TCP_Packet * tempPacket = malloc(sizeof(TCP_Packet));
                        tempPacket = vector_get(&cwnd, i);
                        
                       
                        
                        
                        // We will check to see if the current packet is the one we have received from the client's ACK
                        if (tempPacket->seqNum == ackPacket->origSeqNum)
                        {
                            // We confirm that the client has received our packet, so we remove it from our window
                            vector_delete(&cwnd, i);
                            
                            //for retramission delete the receive packate
                            int temp = 0;
                            for(int k = 1; k < 15; k = k + 3) {
                                long *ack_squence_num = vector_get(&timing, k);
                                if (*ack_squence_num == tempPacket->seqNum) {
                                    temp = k;
                                    break;
                                }
                            }
                            
                            vector_delete(&timing, temp); //delete package sequence number
                            vector_delete(&timing, temp); //dete packate time
                            vector_delete(&timing, temp-1);
                            
                            // Since we remove a packet from our window, we can now add a new packet to our window
                            // and send it to the client
                            if (tempFileSize > 0)
                            {
                                
                                TCP_Packet * packet = malloc(sizeof(TCP_Packet));
                                packet->seqNum = expectedSeqNum;
                                packet->ackNum = 0; // ackPacket->seqNum + MAX_PKT_LENGTH;
                                packet->SYN = 0;
                                packet->FIN = 0;
                                // add more packet values here
                                if (tempFileSize <= PAYLOAD_SIZE)
                                {
                                    packet->PKG_TYPE = 1;
                                    fread(buffer, 1, tempFileSize, fd);
                                    packet->payload_size = tempFileSize;
                                    tempFileSize = 0;
                                }
                                else
                                {
                                    packet->PKG_TYPE = 0;
                                    fread(buffer, 1, PAYLOAD_SIZE, fd);
                                    packet->payload_size = PAYLOAD_SIZE;
                                    tempFileSize -= PAYLOAD_SIZE;
                                }
                                memcpy(packet->payload, buffer, sizeof(buffer));
                                
                                // Add it to the window and update the expected sequence number.
                                vector_add(&cwnd, packet);
                                expectedSeqNum += 1024;
                                // Send the next packet to the client
                                if (sendto(sockfd, packet, sizeof(*packet), 0, (const struct sockaddr *) &clientaddr, clientlen) != -1)
                                {
                                    printf("Sending packet %d 5120\n", packet->seqNum);
                                }
                            
                                //after sending the package, restore the package for retramission
                                *current_time = sys_timestamp(); //get current time
                                vector_add(&timing, current_time);
                                vector_add(&timing, &packet->seqNum);
                                vector_add(&timing, &packet->ackNum);
                                
                            }
                            break;
                        }
                    }
                }
                
            }
            // Here, we will insert code to take care of retransmission
        }
    }
    
}


