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
 
#define SERVER "129.120.151.96"
#define BUFLEN 1023  //Max length of buffer
#define PORT 6969   //The port on which to send data
 
void die(char *s)
{
    perror(s);
    exit(1);
}
 
int main(int argc, char *argv[])
{
    //port number command line arguement
    int port = strtol(argv[1], NULL, 10);

    struct sockaddr_in si_other;
    int sockfd, i=0, slen=sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];
 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
        die("socket");
 
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(port);
     
    if (inet_aton(SERVER , &si_other.sin_addr) == 0) 
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
 
    struct segment {
        unsigned short int source;
	unsigned short int dest;
	unsigned short int length;
	unsigned short int checkSum[16];
	char data[];
    };

    //Send test message
        bzero(message, sizeof(message));
	strcpy(message,"hello\n");
		//send the message
        if (sendto(sockfd, message, strlen(message), 0, (struct sockaddr *) &si_other, slen) == -1)
        {
            die("sendto()");
        }

        //Clear the message buffer and accept the message to be sent
	struct segment seg;


         
        //receive a reply and print it
        //clear the buffer by filling null, it might have previously received data
        bzero(buf, sizeof(buf));
        //try to receive some data, this is a blocking call
        if (recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1)
        {
            die("recvfrom()");
        }
         
        printf("Received message from the server: ");
		printf("%s\n", buf);
	
	//Build UDP Segment
	
	//client receives source port from server
	//adds to source field of segment
	seg.source = atoi(buf);
	
	//Destination is referenced in connection
	seg.dest = atoi(argv[1]);

	//Checksum
	memset((void *)&seg.checkSum,0,sizeof(seg.checkSum));
	
	//Payload
	FILE *fp = fopen(argv[2], "r");
	if (!fp)
	    perror(argv[2]),exit(1);
	
	//Get size of file
	fseek(fp,0L,SEEK_END);
	long fileLength = ftell(fp);
	fseek(fp,0,SEEK_SET);
	
	//Read file into buffer
	char buffer[fileLength];
	fread(buffer,fileLength,1,fp);
	fclose(fp);

	//buffer --> seg.data
	memcpy(seg.data,buffer,fileLength);
	/*
	printf("size of buffer = %zu bytes\n",sizeof(buffer));
	printf("size of seg.data = %d bytes\n",strlen(seg.data));
	*/


	//Length
	/*
	printf("size of seg.source = %zu bytes\n",sizeof(seg.source));
	printf("size of seg.dest = %zu bytes\n",sizeof(seg.dest));
	printf("size of seg.data = %d bytes\n",strlen(seg.data));
	*/

        seg.length = 	sizeof(seg.source)+sizeof(seg.dest)+strlen(seg.data);
	//printf("Truelength size = %d bytes\n",seg.length);

	//Length = sum of headers + data
	//To use length to compute checksum
	unsigned int tmp = seg.length + strlen(seg.data);
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
	//cpy into checksum
	memcpy(seg.checkSum,arr,sizeof(arr));

	//print to client.out
        FILE *out = fopen("client.out","w");	
	fprintf(out,"Source: %d\n",seg.source);
	fprintf(out,"Dest: %d\n",seg.dest);
	fprintf(out,"Length: %d\n",seg.length);

	fprintf(out,"CheckSum: ");
	for(int i=0;i<=15;i++)
	    fprintf(out,"%d",seg.checkSum[i]);
	fprintf(out,"\n\n%s\n",seg.data);

	fclose(out);


	//send segment out
       if(sendto(sockfd, (const void *)&seg, 1024+strlen(seg.data), 0, (struct sockaddr *) &si_other, slen) == -1)
       {
           die("sendto()");
       }


	
    close(sockfd);
    return 0;
}

