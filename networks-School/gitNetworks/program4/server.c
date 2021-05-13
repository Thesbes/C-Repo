/*
    Simple udp server
*/
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <math.h>
#include <time.h>
 
#define BUFLEN 1024  //Max length of buffer
#define PORT 6700   //The port on which to listen for incoming data
 
void die(char *s)
{
    perror(s);
    exit(1);
}

struct segment {
    unsigned short int source;
    unsigned short int dest;
    unsigned int seqNum;
    unsigned int ackNum;
    unsigned short int flags;
    unsigned short int recvWindow; //set to 0
    unsigned short int checkSum;
    unsigned short int urgentDP; //set to 0
    unsigned short int options; //set to 0
    char data[1024]; //set to null
};

void printInfo(struct segment seg)
{
    printf("Source  : %d\n",seg.source);
    printf("Dest    : %d\n",seg.dest);
    printf("seqNum  : %d\n",seg.seqNum);
    printf("ackNum  : %d\n",seg.ackNum);
    printf("flags   : %d\n",seg.flags); 
    printf("recvWin : %d\n",seg.recvWindow);
    printf("checkSum: %d\n",seg.checkSum); 
    printf("urgentDP: %d\n",seg.urgentDP);
    printf("options : %d\n",seg.options);
}

void printOut(struct segment seg)
{
    FILE *fp = fopen("server.out","w"); 
    fprintf(fp,"====SENT=====\n");
    fprintf(fp,"Source  : %d\n",seg.source);
    fprintf(fp,"Dest    : %d\n",seg.dest);
    fprintf(fp,"seqNum  : %d\n",seg.seqNum);
    fprintf(fp,"ackNum  : %d\n",seg.ackNum);
    fprintf(fp,"flags   : %d\n",seg.flags); 
    fprintf(fp,"recvWin : %d\n",seg.recvWindow);
    fprintf(fp,"checkSum: %d\n",seg.checkSum); 
    fprintf(fp,"urgentDP: %d\n",seg.urgentDP);
    fprintf(fp,"options : %d\n",seg.options);
    fprintf(fp,"=============\n\n");
    fclose(fp);
}



unsigned short int computeCksum(struct segment seg)
{
    unsigned short int cksumArr[524];
    unsigned int j,sum=0,cksum,wrap;
    memcpy(cksumArr, &seg, 1048); //cpy 1048 bytes from seg

    //compute sum
    for (j = 0; j<524;j++)
    {
//	printf("0x%04X\n",cksumArr[j]);
        sum = sum + cksumArr[j];
    }

    wrap = sum >> 16; //Wrap around
    sum = sum & 0x0000FFFF;
    sum = wrap + sum;

    wrap = sum >> 16; //Wrap around
    sum = sum & 0x0000FFFF;
    cksum = wrap + sum;

    seg.checkSum = 0xFFFF^cksum;

   // printf("checkSum: %d\n",seg.checkSum);
}
 
int main(int argc, char *argv[])
{
    FILE *fp = fopen("server.out","w");
    fclose(fp);

    srand(time(0));

    //port number command line arguement
    int port = strtol(argv[1], NULL, 10);

    struct sockaddr_in si_me, si_other, cli;
     
    int sockfd, i, slen = sizeof(si_other) , recv_len;
    int listen_fd, conn_fd;
    int len;
    char buf[BUFLEN];
     
    //create a TCP socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        die("socket");
    }
     
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(port);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
    //bind socket to port
    if(bind(sockfd, (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }

    //ready to listen
    if (listen(sockfd,5) != 0)
    {
        printf("Listen Failed.\n");
        exit(0);
    }
    else
        printf("Listening...\n");
    len = sizeof(cli);

    //Accept data packet from client 
    conn_fd = accept(sockfd, (struct sockaddr*)&cli, &len);
    if( conn_fd < 0)
    {
        printf("Connection failed...\n");
	exit(0);
    }
    else
        printf("Connection accepted\n");

    //keep listening for data
    struct segment *seg2 = malloc(sizeof(struct segment));
    struct segment seg;

        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(conn_fd, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
	{
            die("recvfrom()");
	}
        printf("Segment receieved\n");


	//checksum verification
	//

	unsigned short int cksumArr[524];
        unsigned int j,sum=0,cksum,wrap;
	
	memcpy(seg2,buf,sizeof(buf));
        memcpy(cksumArr, seg2, 1048); //cpy 1048 bytes from seg

	/*
        printf("Header\n");
	printf("0x%04X\n", seg2->source);
	printf("0x%04X\n", seg2->dest);
	printf("0x%08X\n", seg2->seqNum);
	printf("0x%08X\n", seg2->ackNum);
	printf("0x%04X\n", seg2->flags);
	printf("0x%04X\n", seg2->recvWindow);
	printf("0x%04X\n", seg2->checkSum);
	printf("0x%04X\n", seg2->urgentDP);
	printf("0x%08X\n", seg2->options);
	*/

        //compute sum
        for (j = 0; j<524;j++)
        {
//	    printf("0x%04X\n",cksumArr[j]);
            sum = sum + cksumArr[j];
        }

        wrap = sum >> 16; //Wrap around
        sum = sum & 0x0000FFFF;
        sum = wrap + sum;

        wrap = sum >> 16; //Wrap around
        sum = sum & 0x0000FFFF;
        cksum = wrap + sum;

        cksum = 0xFFFF^cksum;

        if(cksum != 0)
	{
	    printf("Checksum failed.\n");
	    exit(0);
	}

        //set seg values
	seg.dest = seg2->dest;
	seg.source = seg2->source;
	seg.seqNum = seg2->seqNum;
	seg.flags = seg2->flags;
	seg.recvWindow = seg2->recvWindow;
	//seg.checkSum = seg2->checkSum;
	seg.urgentDP = seg2->urgentDP;
	seg.options = seg2->options;
	strcpy(seg.data, seg2->data);


	//choose rand seq num y, set ackNum x+1;
	//set ACKbit --> 1
	seg.ackNum = seg.seqNum+1;
	seg.seqNum = rand() % 50;
	seg.flags += 16; //Setting ack bit to 1

	//Redo checksum
	unsigned short int cksumArr2[524];
        sum=0;
        memcpy(cksumArr2, &seg, 1048); //cpy 1048 bytes from seg

        //compute sum
        for (j = 0; j<524;j++)
        {
	  //  printf("0x%04X\n",cksumArr2[j]);
            sum = sum + cksumArr2[j];
        }

        wrap = sum >> 16; //Wrap around
        sum = sum & 0x0000FFFF;
        sum = wrap + sum;

        wrap = sum >> 16; //Wrap around
        sum = sum & 0x0000FFFF;
        cksum = wrap + sum;

        seg.checkSum = 0xFFFF^cksum;
	//printf("cksum = %d\n",seg.checkSum);
	printf("==========\n");
	printf("Sending:\n");
	printInfo(seg);
	printOut(seg);
	printf("==========\n");

	
	//Send server acknowledgement
	if(sendto(conn_fd, (const void *)&seg, 1024+strlen(seg.data), 0, (struct sockaddr*) &si_other, slen) == -1)
	{
	  die("sendto()");
	}
	printf("Segment Sent\n");

	bzero(buf,BUFLEN);

	//=================================================
	//This is where things start breaking
	
	//Recieve client ack
        if ((recv_len = recvfrom(conn_fd, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
	{
            die("recvfrom()");
	}
	printf("Segment Received\n");


        struct segment seg3;
	//verify checksum
	//
	sum=0;
	unsigned short int cksumArr3[524];
	memcpy(&seg3, buf, sizeof(buf));
        memcpy(cksumArr, seg2, 1048); //cpy 1048 bytes from seg
        //compute sum
    	for (j = 0; j<524;j++)
    	{
       	    //printf("0x%04X\n",cksumArr3[j]);
            sum = sum + cksumArr[j];
    	}

    	wrap = sum >> 16; //Wrap around
    	sum = sum & 0x0000FFFF;
    	sum = wrap + sum;

    	wrap = sum >> 16; //Wrap around
    	sum = sum & 0x0000FFFF;
    	cksum = wrap + sum;

    	cksum = 0xFFFF^cksum;

        if(cksum != 0)
        {
            printf("Checksum failed.\n");
	    exit(0);
        }
	else
	    printf("Handshake complete!\n");


	//===================================================== 
	//
	//Begin closing
	//
	//Reset
	//Give control back to client
	if(sendto(conn_fd, (const void *)&seg, 1024+strlen(seg.data), 0, (struct sockaddr*) &si_other, slen) == -1)
	{
	  die("sendto()");
	}
	printf("Segment Sent\n");
	printInfo(seg);
	printOut(seg);

	bzero(buf,BUFLEN);

        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(conn_fd, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
	{
            die("recvfrom()");
	}
	printf("Segment Recv\n");
	memset(&seg,0,BUFLEN);
	memset(seg2,0,BUFLEN);
	memcpy(seg2,buf,BUFLEN);

	//verify checksum
	//
	sum=0;
	unsigned short int cksumArr4[524];
    	for (j = 0; j<524;j++)
	    cksumArr4[j] = 0;
        memcpy(cksumArr4, seg2, 1048); //cpy 1048 bytes from seg
        //compute sum
    	for (j = 0; j<524;j++)
    	{
       	    //printf("0x%04X\n",cksumArr4[j]);
            sum = sum + cksumArr4[j];
    	}

    	wrap = sum >> 16; //Wrap around
    	sum = sum & 0x0000FFFF;
    	sum = wrap + sum;

    	wrap = sum >> 16; //Wrap around
    	sum = sum & 0x0000FFFF;
    	cksum = wrap + sum;

    	cksum = 0xFFFF^cksum;

        if(cksum != 0)
        {
            printf("Checksum failed.\n");
	    printf("cksum = %d\n",cksum);
	    //exit(0);
        }

        //set seg values
	seg.dest = seg2->dest;
	seg.source = seg2->source;
	seg.seqNum = seg2->seqNum;
	seg.ackNum = seg2->ackNum;
	seg.flags = seg2->flags;
	seg.recvWindow = seg2->recvWindow;
	seg.checkSum = seg2->checkSum;
	seg.urgentDP = seg2->urgentDP;
	seg.options = seg2->options;

	if (seg.flags - 0x6000 == 1)
	{
	    printf("Fin bit recv.\nSending ack\n");
	    seg.flags += 16; //Ack bit flipped
	}

	
	if(sendto(conn_fd, (const void *)&seg, 1024+strlen(seg.data), 0, (struct sockaddr*) &si_other, slen) == -1)
	{
	  die("sendto()");
	}
	printf("Segment Sent\n");
	printf("==========\n");
	printInfo(seg);
	printOut(seg);
	printf("==========\n");

//Send final closing call

	if(sendto(conn_fd, (const void *)&seg, 1024+strlen(seg.data), 0, (struct sockaddr*) &si_other, slen) == -1)
	{
	  die("sendto()");
	}
	printf("Segment Sent\n");
	printf("==========\n");
	printInfo(seg);
	printOut(seg);
	printf("==========\n");
//recv final closing ack from client

        if ((recv_len = recvfrom(conn_fd, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
	{
            die("recvfrom()");
	}

	printf("Done!\n");

    close(sockfd);
    return 0;
}
