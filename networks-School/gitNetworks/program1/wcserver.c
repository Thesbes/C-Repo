/* Server Code */
/* Runs the server on port 22000 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
 
int main(int argc, char *argv[])
{
    char str[200];
    int listen_fd, conn_fd;
    struct sockaddr_in servaddr;
    char recv_line[200];
    char send_line[200];

    char vowelArray[10] = "aeiouAEIOU";
    char *token;
    int vowelCount;
    int alphabetCount;
    int wc;

    //checks if letter is in alphabet
    int isAlphabet(char letter)
    {
        if((letter >= 'A' && letter <= 'Z') || 
	   (letter >= 'a' && letter <= 'z'))
		return 1;
	return 0;
    }

    //Checks if letter is in list of vowels
    int isVowel(char letter)
    {
        for(int i=0; i<sizeof(vowelArray); i++)
	    if(letter == vowelArray[i])
		return 1;
	return 0;
    }

    //Used to add rest of relevent data to send_message
    //Includes alphabet count, vowel count, word count
    //sprintf to convert int --> "string"
    void infoPool()
    {
	char tmp[50];

	strcat(send_line," words: ");
	sprintf(tmp,"%d",wc);
	strcat(send_line,tmp);

	strcat(send_line,"\n  alphabets: ");
	sprintf(tmp,"%d",alphabetCount);
	strcat(send_line,tmp);

	strcat(send_line,"\n  vowels: ");
	sprintf(tmp,"%d",vowelCount);
	strcat(send_line,tmp);
	
	strcat(send_line,"\n");
    }

    void modify()
    {
        //Capitalize first letter	
        if(token[0] >= 'a' && token[0] <= 'z')
	    token[0] = token[0] - 32;
        //count vowels and alphabet
	for(int i = 0; i<strlen(token); i++)
	{
	    //if isAlphabet is true then check vowel
	    int tmp = alphabetCount;
            if(tmp != (alphabetCount+= isAlphabet(token[i])))
	        vowelCount+=isVowel(token[i]);
	}
    }


    void chat(int sockfd)
    {
	printf("Connection established\n\n");
	int n; //buffer incrementer
        while(1)
	{
            bzero(recv_line,200);
	    if(read(sockfd, recv_line, sizeof(recv_line)) > 0)
	    {
                if(strncmp("quit", recv_line, 4) == 0)
		{
                    printf("'quit' received\nExiting Server . . . \n");
		    break;
		}

                //String maninpulation
	        //acts on recv_line
	        int start; //First char in token
	        const char delim[] = " "; //Delimeter
	        wc = 0; //word count variable increments at each token
		vowelCount=0;
		alphabetCount=0;

		if((token = strtok(recv_line,delim)) != NULL)
		{
		    char tmp[200];
		    //modify token, concat to send_line, wc++
                    modify(token);

		    strcpy(send_line, token);
		    strcat(send_line, " ");
		    wc++;

		    while((token = strtok(NULL,delim)) != NULL)
		    {
		        //modify token, concat to send_line, wc++
			modify(token);
			strcpy(tmp,token);

			//add modified token to sent message
			strcat(send_line,tmp);
                        //replace space in message to send message
		        strcat(send_line, " ");

		        wc++;
		    }
		}
	    }

	    infoPool();

            write(sockfd, send_line, strlen(send_line)); // write to the client
	    bzero(send_line,200);
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

      close (conn_fd); //close the connection
      }
