/* Client Code */
/* Connect to port 22000 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
 
int main (int argc, char **argv)
{
    int sockfd, n;
    int len = sizeof(struct sockaddr);
    char recv_line[40960];
    char send_line[40960];
    struct sockaddr_in servaddr;

    void chat(int sockfd)
    {
	int n; //buffer incrementer
        while(1)
	{
            //Record user input in send_line until 'Enter'
	    printf("Client: ");
	    n=0;
	    while((send_line[n++] = getchar()) != '\n');

            if (strncmp("quit", send_line, 4) == 0)
	    {
	        write(sockfd, send_line, strlen(send_line));
		printf("Exiting . . .\n");
		break;
	    }
	    write(sockfd, send_line, strlen(send_line));
            bzero(send_line,200);

	    if(read(sockfd, recv_line, sizeof(recv_line)) > 0)
	    {
                printf("  Server: %s",recv_line); // print the received text from server
                bzero(recv_line,200);
	    }
	}
    }


    /* AF_INET - IPv4 IP , Type of socket, protocol*/
    sockfd=socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr,sizeof(servaddr));
 

    //port number command line arguement
    int port = strtol(argv[1],NULL,10);

    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(port); // Server port number
 
    /* Convert IPv4 and IPv6 addresses from text to binary form */
    inet_pton(AF_INET,"129.120.151.94",&(servaddr.sin_addr));
    
 
    /* Connect to the server */
    if(connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) != 0)
    {
        printf("Sever connect failed\nExiting . . .\n");
	exit(0);
    }
    else
        printf("Connection established at port %d\n",port);

    chat(sockfd);
    close(sockfd);
}
