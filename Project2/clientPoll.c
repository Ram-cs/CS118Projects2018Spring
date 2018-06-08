#include "support.h"
#include "vector.h"

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

    // Set up the server IP
    struct sockaddr_in serveraddr;
    memset((char *) &serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(portno);
    serveraddr.sin_addr.s_addr = INADDR_ANY;

    socklen_t serverlen = sizeof(serveraddr);
    int n;

    // Make the Packet sending requested filename
    TCP_Packet Request_Packet;
    Request_Packet.seqNum = 0;
    Request_Packet.ackNum = 0;
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
    int expected_SEQ_NUM = 0;

    // Setting up the poll
    struct pollfd thePoll[1];
    thePoll[0].fd = sockfd; 
    thePoll[0].events = POLLIN;
    
    // Setting up the file buffer
    vector fileBuffer;
    vector_init(&fileBuffer);

    // While there is still data to be received from the server...
    while(finish_file == 0)
    {       
        int r = poll(thePoll, 1, -1);
        if (r > 0)
        {
            // If there are data packets to be read in from the socket
            if (thePoll[0].revents & POLLIN) 
            { 
                // Read from it.
                TCP_Packet * Result_Packet = malloc(sizeof(TCP_Packet));
                n = recvfrom(sockfd, Result_Packet, sizeof(*Result_Packet), 0, (struct sockaddr *) &serveraddr, &serverlen);
                printf("Receiving packet %d\n", Result_Packet->seqNum);

                // If it is the data packet with the expected sequence number:
                if (Result_Packet->seqNum == expected_SEQ_NUM)
                {
                    // Write the data to our file
                    fwrite(Result_Packet->payload, Result_Packet->payload_size, 1, fp);
                    // Set the next expected sequence number.
                    expected_SEQ_NUM += 1024;
                    // If this is the last packet, prepare to exit the event loop
                    if (Result_Packet->PKG_TYPE == 1)
                        finish_file = 1;
                    // If there are currently packets in our buffer
                    if (fileBuffer.total != 0)
                    {
                        // Sort the buffer
                        for (int i = 0; i < fileBuffer.total; i++)
                        {
                            for (int j = i + 1; j < fileBuffer.total; j++)
                            {
                                TCP_Packet * left = malloc(sizeof(TCP_Packet));
                                TCP_Packet * right = malloc(sizeof(TCP_Packet));
                                left = vector_get(&fileBuffer, i);
                                right = vector_get(&fileBuffer, j);
            
                                if (left->seqNum > right->seqNum)
                                {
                                    vector_set(&fileBuffer, i, right);
                                    vector_set(&fileBuffer, j, left);
                                }
                            }
                        }
                        // While there are still packets in our buffer...
                        while (fileBuffer.total != 0)
                        {
                            TCP_Packet * first_packet = malloc(sizeof(TCP_Packet));
                            first_packet = vector_get(&fileBuffer, 0);

                            if (first_packet->seqNum == expected_SEQ_NUM)
                            {
                                fwrite(first_packet->payload, first_packet->payload_size, 1, fp);
                                expected_SEQ_NUM += 1024;
                                vector_delete(&fileBuffer, 0);
                            }
                            else
                                break;
                        }
                    }
                    // Here, we create the ACK packet to send back to the client
                    TCP_Packet ackPacket;
                    ackPacket.seqNum = expected_SEQ_NUM;
                    ackPacket.ackNum = expected_SEQ_NUM;
                    ackPacket.origSeqNum = Result_Packet->seqNum;
                    
                    n = sendto(sockfd, (struct TCP_Packet *) &ackPacket, sizeof(ackPacket), 0, (const struct sockaddr *) &serveraddr, serverlen);
                    if (n != -1) 
                    {
                        printf("Sending packet %d\n", ackPacket.seqNum);
                    }
                }
                // Else if it is not a data packet with the expected sequence number...
                else
                {
                    // Add the packet to the buffer
                    vector_add(&fileBuffer, Result_Packet);
                    // Send an ACK with the same expected sequence number
                    TCP_Packet ackPacket;
                    ackPacket.seqNum = expected_SEQ_NUM;
                    ackPacket.ackNum = expected_SEQ_NUM;
                    ackPacket.origSeqNum = Result_Packet->seqNum;
                    
                    n = sendto(sockfd, (struct TCP_Packet *) &ackPacket, sizeof(ackPacket), 0, (const struct sockaddr *) &serveraddr, serverlen);
                    if (n != -1) 
                    {
                        printf("Sending packet %d\n", ackPacket.seqNum);
                    }

                }
                
            }
        }
        
    }

}