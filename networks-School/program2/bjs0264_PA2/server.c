/* Server Code */
/* Runs the server on port 22000 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h> 
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    char str[200];
    int listen_fd, conn_fd;
    int outSockfd;
    struct sockaddr_in servaddr, outaddr;
    char recv_line[200];
    char send_line[1000000];
    struct hostent *hinfo; //Host info
    char ip[16];
    int count =0;

    struct in_addr a;


    void chat(int sockfd)
    {
	printf("Connection established\n\n");
	int n; //buffer incrementer
        while(1)
	{
	    char host[200];
	    char path[200];
	    int hostResolved = 0;
	    count=0;
	    while(hostResolved == 0)
	    {
                bzero(recv_line,200);
	        if(read(sockfd, recv_line, sizeof(recv_line)) > 0)
                    if(strncmp("quit", recv_line, 4) == 0)
		    {
                        printf("'quit' received\nExiting Server . . . \n");
                        close (conn_fd); //close the connection
			exit(1);
		    }
		printf("recvline: %s###\n",recv_line);

	        /* Form GET request based on input*/
	        for(int i = 0; i<sizeof(recv_line); i++)
	        {
                    /*path*/
	            if(recv_line[i] == '/')
		    {
		        for (int j = i; j<sizeof(recv_line); j++)
		        {	
			    //replace newline w/ zero to remove from string
		            if(recv_line[j] != '\n')
		                path[j-i] = recv_line[j];
			    else
		                path[j-i] = 0;
		        }
	                host[i] = 0;
		        break;
		    }
	            host[i] = recv_line[i];
	        }

	        /*hostname --> ip*/
		//if not null, check if its in the cache
		//if it's not, resolve the host as normal.
	        if((hinfo = gethostbyname(host)) == NULL)
	        {
		    char usage[200];
	            printf("Error resolving host.\n");
		    sprintf(usage,"Error resolving host.\nCorrect form: www.hostname/path\n");
		      write(sockfd,usage,sizeof(usage));
	        }
		else
		{
		    FILE *bruhfp;
		    if((bruhfp = fopen("list.txt","r")))
		    {
		        hostResolved = 1;
			char word[100];
		        while((fgets(word,100,bruhfp) != NULL))
			{
			    printf("word = %s\n",word);
			    strtok(word," ");
			    //if in list return cache
			    if (strcmp(host,word) == 0)
			    {
				FILE *readFrom = fopen(host,"r");
				char cache[300000];
				char line[10000];
				printf("Returning from cache\n");
				while(fscanf(readFrom,"%s",line) != EOF)
				    strcat(cache,line);

				printf("%d",sizeof(cache));

				write(sockfd,cache,sizeof(cache));
				bzero(cache,sizeof(cache));
				printf("Cache returned\n");
				hostResolved=0;
			    }
			    count++;
			    printf("count = %d\n",count);
			}
		    }
		}
	    }
	    /*Get statement*/
	    char GET[200] = {0};
	    sprintf(GET,"GET %s HTTP/1.1\r\nHost: %s\r\nContent-Type: text/plain\r\n\r\n",path,host);
	    //sprintf(GET,"GET %s HTTP/1.1\r\nHost: %s\r\n\r\n",path,host);
	    printf("%s\n",GET);

	    /*IP assignment*/
            bcopy(*hinfo->h_addr_list++, (char *) &a, sizeof(a));
            sprintf(ip,"%s",inet_ntoa(a));
		
	    printf("%s\n",ip);

	    outSockfd=socket(AF_INET, SOCK_STREAM,0);
	    outaddr.sin_family=AF_INET;
	    outaddr.sin_port=htons(80);

	    /*Text to binary ip*/
	    inet_pton(AF_INET,ip,&(outaddr.sin_addr));

	    /*Connect to the server*/
	    if(connect(outSockfd,(struct sockaddr *)&outaddr,sizeof(outaddr)) != 0)
	    {
	        printf("Failed to connect to port 80.\n");
	    }
	    else
	    {
	        printf("Connection established at port 80\n");
	        FILE *fp;

	        char word[100];
		int count=0; //number of lines in file

		//Write GET out to server
		write(outSockfd,GET,strlen(GET));
		printf("Request sent out port 80\nWaiting for response\n");

		//Wait for response
		if(read(outSockfd, send_line,sizeof(send_line)) > 0)
		{
		    printf("Response Received\n");

		    //Get timestamps and request codes
		    //=============================================
                    char tmp[sizeof(send_line)];
		    strcpy(tmp,send_line);
		    char delim[3] = " \n";

		    char *token =malloc(sizeof(tmp)+100);
		    char *tokenarray[100] = {NULL};
		    int pos = 0; //array incrementer
		    token = strtok(tmp,delim);

		    //get requestCode (0)
	            token = strtok(NULL,delim);
		    tokenarray[pos]=token;
		    pos++;

		    //skip this nonsense

                    while(strcmp(token,"Date:") != 0)
	                token = strtok(NULL,delim);
		    //Skip day of week
		    token = strtok(NULL,delim);

		    //Day (1) --> Complete
	            token = strtok(NULL,delim);
		    tokenarray[pos]=token;
		    pos++;

		    //Month (2) --> Complete
	            token = strtok(NULL,delim);

		    if(strcmp(token,"Jan") ==      0)
		        tokenarray[pos] = "01";
		    else if(strcmp(token,"Feb") == 0)
		        tokenarray[pos] = "02";
		    else if(strcmp(token,"Mar") == 0)
		        tokenarray[pos] = "03";
		    else if(strcmp(token,"Apr") == 0)
		        tokenarray[pos] = "04";
		    else if(strcmp(token,"May") == 0)
		        tokenarray[pos] = "05";
		    else if(strcmp(token,"Jun") == 0)
		        tokenarray[pos] = "06";
		    else if(strcmp(token,"Jul") == 0)
		        tokenarray[pos] = "07";
		    else if(strcmp(token,"Aug") == 0)
		        tokenarray[pos] = "08";
		    else if(strcmp(token,"Sep") == 0)
		        tokenarray[pos] = "09";
		    else if(strcmp(token,"Oct") == 0)
		        tokenarray[pos] = "10";
		    else if(strcmp(token,"Sep") == 0)
		        tokenarray[pos] = "11";
		    else if(strcmp(token,"Oct") == 0)
		        tokenarray[pos] = "12";
		    pos++;

		    //Year (3) --> complete
	            token = strtok(NULL,delim);
		    tokenarray[pos]=token;
		    pos++;

		    //HH:MM:SS (4) --> Complete
	            token = strtok(NULL,delim);
		    char tmpToken[9];
		    for(int i,j=0; i<strlen(token);i++)
		    {
		        if(token[i] == ':'){}
			else
			{
			    tmpToken[j] = token[i];
			    j++;
			}
		    }
		    tokenarray[pos]=tmpToken;

		    //Order :3214


		    //concat final token
		    char finalToken[14] = {};
	            strcat(finalToken,tokenarray[3]);
	            strcat(finalToken,tokenarray[2]);
	            strcat(finalToken,tokenarray[1]);
	            strcat(finalToken,tokenarray[4]);
		    //=============================================


                    printf("Sending webpage\n");
                    write(sockfd, send_line, strlen(send_line)); // write to the client


		    /*List of objectives
		     * 1.) Check hostname in list.txt
		     *       if in list return cache
		     *       else request webserver
		     *           if responseCode =200 get request time
		     *               if count<5 add to list
		     *               else replace with lowest cache in list
		     *                   cache-->file
		     *                   delete replacedFile
		     *           return to client
		     * 2.) 
		     * */

	            if(strcmp(tokenarray[0],"200") == 0)
		    {
			printf("Count = %d",count);
			if(count>=5)
			{
		            printf("MADE IT\n");
			}
	                /*Cache it*/
	                fp = fopen("list.txt","a");
	                fprintf(fp,"%s %s\n",host,finalToken);
	                fclose(fp);

		        //Cache-->file
		        fp = fopen(host,"w");
		        fprintf(fp,"%s",send_line);
		        fclose(fp);
		    }

		    bzero(send_line,1000000);
		    printf("\nDONE\n");
		    close(outSockfd);
		}
		else
		    printf("Read from port 80 unsuccesful!\n");
	    }
	}
    }

 
    /* AF_INET - IPv4 IP , Type of socket, protocol*/
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
 
    bzero(&servaddr, sizeof(servaddr));

    //port number command line arguement
    int port = strtol(argv[1], NULL, 10);
 
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(port);
 
    /* Binds the above details to the socket */
    bind(listen_fd,  (struct sockaddr *) &servaddr, sizeof(servaddr));
	/* Start listening to incoming connections */
    listen(listen_fd, 10);

      /* Accepts an incoming connection */
      printf("Server up at port %d\n",port);
      conn_fd = accept(listen_fd, (struct sockaddr*)NULL, NULL);
      printf("Listen successful\n");
      chat(conn_fd);
}

