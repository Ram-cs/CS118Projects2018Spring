
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
    SYN_Packet.ackNum = 2000;
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
    Request_Packet.ackNum = SYNACK_Packet->seqNum + MAX_PKT_LENGTH;
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

    TCP_Packet * Result_Packet = malloc(sizeof(TCP_Packet));
    n = recvfrom(sockfd, Result_Packet, sizeof(*Result_Packet), 0, (struct sockaddr *) &serveraddr, &serverlen);
    printf("Receiving packet %d\n", Result_Packet->ackNum);
    printf("Received File Content: %s\n", Result_Packet->payload);


















    //SENDING THE PACKAGES TO SERVER
    
    //    int slen=sizeof(serveraddr);
    //    for(int i = 0; i < NUM_PKG; i++) {
    //        printf("Sending packet %d\n", i);
    //        sprintf(Request_Packet.payload, "This is packet %d\n", i);
    ////        printf("***%s",Request_Packet.payload );
    //        if (sendto(sockfd, Request_Packet.payload, MAX_PKT_LENGTH, 0, (const struct sockaddr *) &serveraddr, slen)== -1) {
    //            error("sendto() error");
    //        }
    //
    //    }
    
    
    //DIVIDE FILE INTO SMALL CHUNK, meaning read file upto the allowed size and repeat the process
    /*
    FILE* fd = NULL;
    char buffer[MAX_PKT_LENGTH]; //1024
    size_t byte_read = 0;
    fd = fopen(Request_Packet.payload, "r"); //open the client file
    
    if(fd == NULL) { //if file is empty then exit
        error("File is empty");
    }
    
    fseek(fd, 0 , SEEK_END);
    long fileSize = ftell(fd);
    fseek(fd, 0 , SEEK_SET);
    */
   
    /*
    if (fileSize <= 1008) { //if the size of fd is less or equal to 1008
        fread(buffer, 1, fileSize, fd);
        printf("Sending packet %d\n", Request_Packet.seqNum); //output sequence number
        Request_Packet.seqNum += Request_Packet.seqNum + fileSize; //increase the sequence number by file size
        if (sendto(sockfd, buffer, MAX_PKT_LENGTH, 0, (const struct sockaddr *) &serveraddr, serverlen)== -1) {
            error("sendto() error");
        }
    } else {
        while(1) {
            byte_read = fread(buffer, 1, 1008, fd); //loop untill file is greater than 1008
            printf("Sending packet %d\n", Request_Packet.seqNum); //output sequence number
            Request_Packet.seqNum += Request_Packet.seqNum + 1008; //increase the sequence number by 1008
            if (sendto(sockfd, buffer, MAX_PKT_LENGTH, 0, (const struct sockaddr *) &serveraddr, serverlen)== -1) {
                error("sendto() error");
            }
            
            if (byte_read <= 0) { //if read all files then exit the loop
                break;
            }
            
            if (byte_read <= 1008) { //if file is less than 1008 then break
                fread(buffer, 1, byte_read, fd);
                printf("Sending packet %d\n", Request_Packet.seqNum); //output sequence number
                Request_Packet.seqNum += Request_Packet.seqNum + byte_read; //increase the sequence number by file size
                if (sendto(sockfd, buffer, MAX_PKT_LENGTH, 0, (const struct sockaddr *) &serveraddr, serverlen)== -1) {
                    error("sendto() error");
                }
                break;
            }
            
        }
    }
    */
    //    if (fd != NULL) { //check if the file is empty
    //        // read up to sizeof(buffer) bytes
    //        while ((byte_read = fread(buffer, 1, 1008, fd)) > 0)
    //        {
    //            // process bytesRead worth of data in buffer
    //            printf("Sending packet DATA:: %s\n", buffer);
    //            if (sendto(sockfd, buffer, MAX_PKT_LENGTH, 0, (const struct sockaddr *) &serveraddr, slen)== -1) {
    //                error("sendto() error");
    //            }
    //        }
    //    }
    
    //fclose(fd);
    
    
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
