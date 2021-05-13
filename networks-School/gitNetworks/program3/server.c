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
 
#define BUFLEN 512  //Max length of buffer
#define PORT 6700   //The port on which to listen for incoming data
 
void die(char *s)
{
    perror(s);
    exit(1);
}
 
int main(int argc, char *argv[])
{

    //port number command line arguement
    int port = strtol(argv[1], NULL, 10);

    struct sockaddr_in si_me, si_other;
     
    int sockfd, i, slen = sizeof(si_other) , recv_len;
    char buf[BUFLEN];
     
    //create a UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
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


        struct segment {
        unsigned short int source;
	unsigned short int dest;
	unsigned short int length;
	unsigned short int checkSum[16];
	char data[];
    };

	struct segment *seg = malloc(sizeof(struct segment));
    //keep listening for data
    while(1)
    {


	memset(seg,0,sizeof(seg));
	//struct segment seg;
        printf("Waiting for data...\n");
		fflush(stdout);
        bzero (buf, BUFLEN);        
        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
        {
            die("recvfrom()");
        }
         
        //print details of the client/peer and the data received
        printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
        //printf("Received Data: %s\n" , buf);

        unsigned short int sourcePort = ntohs(si_other.sin_port);
	bzero (buf,BUFLEN);
	sprintf(buf,"%d",sourcePort);
         
        //now reply the client with the same data
        if (sendto(sockfd, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
        {
            die("sendto()");
        }


	int buffle = 1024+strlen(seg->data);
	    bzero(buf, BUFLEN);
        if ((recv_len = recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
        {
            die("recvfrom()");
        }
	memcpy(seg,buf,sizeof(buf));

	//Length = sum of headers + data
	//To use length to compute checksum
	unsigned int tmp = seg->length + strlen(seg->data);
	unsigned short int arr[16];
	memset(arr,0,sizeof(arr));
	unsigned int j = 15;
	//printf("Tmp = %d\n",tmp);

	//turn into binary
	for(int i = 0; i<=15; i++)
	{
	    unsigned int POW = (unsigned int)pow(2,j);
	    //printf("tmp: %d, j: %d, POW: %d\n",tmp, j,POW);
	    if(tmp>=POW)
	    {
	        arr[i] = 1;
		tmp -= POW;
	    }
	    j--;
	}

	//flip binary
	for(int i = 0; i<=15;i++)
	{
	    if(arr[i] == 1)
	        arr[i] = 0;
	    else
	        arr[i] = 1;
	}
        
	//if checksums aren't the same, print error
	int isTrue = 1;
	for(int i = 0; i<=15; i++)
	{
	    if(arr[i] != seg->checkSum[i])
	    {
	        isTrue = 0;
	        break;
	    }
	}

	if(isTrue == 1)
	{
	    //print what was received
	    
            FILE *out = fopen("server.out","w");	
	    fprintf(out,"Source: %d\n",seg->source);
	    fprintf(out,"Dest: %d\n",seg->dest);
	    fprintf(out,"Length: %d\n",seg->length);

	    fprintf(out,"CheckSum: ");
	    for(int i=0;i<=15;i++)
	    {
	        fprintf(out,"%d",seg->checkSum[i]);
	    }
	    fprintf(out,"\n\n%s\n",seg->data);
	    fclose(out);
	}
	else
	    printf("ERROR: Checksum incorrect. Packet dropped\n");

	memset((void *)seg->checkSum,0,sizeof(seg->checkSum));


    }
 
    close(sockfd);
    return 0;
}

