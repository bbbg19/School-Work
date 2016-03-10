/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{	int doneTranslating = 0;
	int continueRead = 0; 
	int size =0 ;
	char a;
    int sockfd, portno;
    socklen_t clilen;
    char buffer[1000];
	char keyFile[1000];
    struct sockaddr_in serv_addr, cli_addr;
    int n,i;
    if (argc < 2) {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
       error("ERROR opening socket");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    //user argument for port no
	portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
	//bind server socket to port 
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
		error("ERROR on binding");
    //server now starts listening up to 5 attempts
	listen(sockfd,5);
     clilen = sizeof(cli_addr); //get size of client address structure   
	
	 
	 
	 //add a while loop so server continually operates
	 while(1){				  
		 
		int newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) 
			  error("ERROR on accept");
		  
		int pid = fork();
		if(pid<0){
			error("Error on fork");
		}
		  //child case, close connection to old socket
		if(pid == 0){
			char  inputLength[20];
			close(sockfd);
			bzero(buffer,1000);		
			
			//enter 2nd to c while loop until we encounter \\ to terminate 
			while (doneTranslating ==0){			
				bzero(buffer,1000);
				bzero(keyFile,1000);
	
				//read in continue status
				n= read(newsockfd,buffer,1);
				if(buffer[0]=='@'){
					doneTranslating =1;
					
				}
				
				//read in textfile
				
				n = read(newsockfd,buffer,999);
				if (n < 0) 
					error("ERROR reading from socket");
		
				sprintf(inputLength,"%d", n);
				//send 0 for success
				n = write(newsockfd,inputLength,3);
				if (n < 0) 
					error("ERROR writing to socket");
				
				//read in keyfile
				n=read(newsockfd, keyFile, 999);
				sprintf(inputLength,"%d", n);
				
				//go through contents of read in file	
				for(i=0; i<strlen(buffer); i++){
					a = buffer[i];
					//check inputchar is a valid letter
					if(!isalpha(a) && !isspace(a) && buffer[i]!= '@'){
						printf("improper letter: %c\n", a);
						n = write(newsockfd,"Inproper letter: ",strlen(buffer));
						exit(0);
					}		
		
					//check if newline character
					else if(buffer[i] =='\n' || buffer[i] =='\0' || buffer[i] ==0 ){
						//printf("\nSkipping newline\n");	
						break;
					}
					else if(buffer[i] == ' ' && keyFile[i] =='A'){
						buffer[i]= ' ';
						size++;
					}
					else if(buffer[i] == ' '){					 
						buffer[i]= (26- (keyFile[i]-'A'))%27 +'A';
						size++;
					}
									
					//section for converting cipher text to regular text
					else {
						
						a=((buffer[i]-'A') - (keyFile[i]-'A'))%27;
						if(a<0)
							a+=27;
									
						if(a==26){
							buffer[i]= ' ';							
						}
						else{
							a=a+'A';					
							buffer[i]= a;
						}
						size++;
					}					
				}
				n = write(newsockfd,buffer,size);
				if (n < 0) 
					error("ERROR writing to socket");				
				size=0;
				}
				exit (0);
			}

		  else{
			  close(newsockfd);
		 }		 
     }	 

     close(sockfd);
     return 0; 
}
