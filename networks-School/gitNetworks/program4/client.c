/*
    Simple udp client
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <math.h>
#include <time.h>
 
#define SERVER "129.120.151.96" //cse03.cse.unt.edu
#define BUFLEN 1024  //Max length of buffer
#define PORT 6969   //The port on which to send data
 
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
    unsigned int options;
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
    FILE *fp = fopen("client.out","a"); 
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
 
int main(int argc, char *argv[])
{
    FILE *fp = fopen("client.out","w");
    fclose(fp);


    srand(time(0));

    //port number command line arguement
    int port = strtol(argv[1], NULL, 10);

    struct sockaddr_in si_other;
    int sockfd, i=0, slen=sizeof(si_other), recv_len;
    char buf[BUFLEN];
    char message[BUFLEN];
 
    //socket creation
    if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        die("socket");
 
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_addr.s_addr = inet_addr(SERVER); 
    si_other.sin_port = htons(port);

    if(connect(sockfd,(struct sockaddr*)&si_other, slen) != 0)
    {
        printf("Failed to connect.\n");
	exit(0);
    }
    else
	printf("Connected!\n");

    //Get source port 
    getsockname(sockfd,(struct sockaddr*)&si_other,&slen);

    struct segment seg;
    struct segment *seg2 = malloc(sizeof(struct segment));
    //Do segment stuff
    seg.dest = port; 
    seg.source = ntohs(si_other.sin_port);
    seg.seqNum = rand()%50;
    seg.ackNum = 0;
    seg.flags = 0x6000; //Header length
    seg.recvWindow = 0;
    seg.checkSum = 0; //to compute
    seg.urgentDP = 0;
    seg.options = 0;
    bzero(seg.data,1024);

    //Set SYNbit to 1
    seg.flags += 2; //turns second to last bit on



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

    //printf("checkSum: %d\n",seg.checkSum);

    printf("==========\n");
    printf("Sending:\n");
    printInfo(seg);
    printOut(seg);
    printf("==========\n");

    // Cli attempts con
    printf("Sending segment...\n");
    if(recv_len = sendto(sockfd, (const void *)&seg, 1024+strlen(seg.data), 0, (struct sockaddr *) &si_other, slen) == -1)
    {
       die("sendto()");
    }
    
    bzero(buf,sizeof(buf));

    //Receive server Ack
    if(recv_len = recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1)
    {
        die("recvfrom()");
    }
    printf("Segment receieved\n");

    unsigned short int cksumArr2[524];
    memcpy(seg2,buf,sizeof(buf));

    //printInfo(seg2);
    memcpy(cksumArr2,seg2,1048);

    //Checksum segment
    //compute sum
    sum = 0;
    for (j = 0; j<524;j++)
    {
//	printf("0x%04X\n",cksumArr[j]);
        sum = sum + cksumArr2[j];
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

    if(seg2->flags - seg.flags == 16)
        printf("ACKbit set\n");
    else
    {
        printf("ACKbit unset\nExiting...\n");
	exit(0);
    }

    //set seg values
    seg.dest = seg2->dest;
    seg.source = seg2->source;
    seg.seqNum = seg2->ackNum;   //seqNum = x+1 or ackNum
    seg.ackNum = seg2->seqNum+1; //ackNum=y+1
    seg.flags = seg2->flags;
    seg.recvWindow = seg2->recvWindow;
    //seg.checkSum = seg2->checkSum;
    seg.urgentDP = seg2->urgentDP;
    seg.options = seg2->options;
    strcpy(seg.data, seg2->data);

    //Redo checksum
    unsigned short int cksumArr3[524];
    sum=0;
    memcpy(cksumArr3, &seg, 1048); //cpy 1048 bytes from seg

    //compute sum
    for (j = 0; j<524;j++)
    {
        //  printf("0x%04X\n",cksumArr2[j]);
        sum = sum + cksumArr3[j];
    }

    wrap = sum >> 16; //Wrap around
    sum = sum & 0x0000FFFF;
    sum = wrap + sum;

    wrap = sum >> 16; //Wrap around
    sum = sum & 0x0000FFFF;
    cksum = wrap + sum;

    seg.checkSum = 0xFFFF^cksum;
//    printf("cksum = %d\n",seg.checkSum);
    printf("==========\n");
    printf("Sending:\n");
    printInfo(seg);
    printOut(seg);
    printf("==========\n");
    
    //Send client ack
    if(sendto(sockfd, (const void *)&seg, 1024+strlen(seg.data), 0, (struct sockaddr *) &si_other, slen) == -1)
    {
       die("sendto()");
    }
    printf("Sending segment\n");

    if(recv_len = recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1)
    {
        die("recvfrom()");
    }
    printf("Segment receieved\n\n\n");

    
    memset(&seg,0,1048);
    //Get ready to close
    //Building segment for close
    seg.dest = port; 
    seg.source = ntohs(si_other.sin_port);
    //seg.seqNum = rand();
    seg.seqNum = 1024;
    seg.ackNum = 512;
    seg.flags = 0x6001; //Header length + FINBIT
    seg.recvWindow = 0;
    seg.checkSum = 0; //to compute
    seg.urgentDP = 0;
    seg.options = 0;
    bzero(seg.data,1024);

    //Redo checksum
    bzero(cksumArr3,0);
    sum=0;
    memcpy(cksumArr3, &seg, 1048); //cpy 1048 bytes from seg

    //compute sum
    for (j = 0; j<524;j++)
    {
        //  printf("0x%04X\n",cksumArr2[j]);
        sum = sum + cksumArr3[j];
    }

    wrap = sum >> 16; //Wrap around
    sum = sum & 0x0000FFFF;
    sum = wrap + sum;

    wrap = sum >> 16; //Wrap around
    sum = sum & 0x0000FFFF;
    cksum = wrap + sum;

    seg.checkSum = 0xFFFF^cksum;
    printf("==========\n");
    printf("Sending:\n");
    printInfo(seg);
    printOut(seg);
    printf("==========\n");

    if(sendto(sockfd, (const void *)&seg, BUFLEN, 0, (struct sockaddr *) &si_other, slen) == -1)
    {
       die("sendto()");
    }
    printf("Sending segment\n");


    if(recv_len = recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1)
    {
        die("recvfrom()");
    }
    printf("Segment receieved\n\n\n");

    memset(seg2,0,BUFLEN);
    memcpy(seg2,buf,BUFLEN);
    unsigned short int cksumArr4[524];

    //printInfo(seg2);
    memcpy(cksumArr4,seg2,1048);

    //Checksum segment
    //compute sum
    sum = 0;
    for (j = 0; j<524;j++)
    {
	printf("0x%04X\n",cksumArr4[j]);
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
        printf("Checksum failed\n");
	printf("cksum = %d\n",cksum);

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

	printf("Close request acknowledge from server recv\n");
	printInfo(seg);
	printOut(seg);

//last close call from server
    if(recv_len = recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1)
    {
        die("recvfrom()");
    }
    printf("Segment receieved\n\n\n");

    memcpy(seg2,buf,BUFLEN);
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

	printInfo(seg);
	printOut(seg);

    if(sendto(sockfd, (const void *)&seg, BUFLEN, 0, (struct sockaddr *) &si_other, slen) == -1)
    {
       die("sendto()");
    }
    printf("Sending segment\n");

    close(sockfd);
}
