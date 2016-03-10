#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h> 

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
	int i;
	int fileRead = 0; 
    int sockfd, portno, n, textFileSize;
    struct sockaddr_in serv_addr;
    struct hostent *server;
	char test123 = 'a';
    char keyBuffer[1000];
	char textBuffer[1000];
    if (argc < 4) {
       fprintf(stderr,"usage %s plaintext key port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[3]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname("localhost");
	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }	
	
	//open text file and read in input 
	int textFile = open(argv[1], O_RDONLY);
	int keyFile  = open(argv[2], O_RDONLY);
	if(keyFile<0){
		printf("Error Opening KeyFile");
		exit(1);
	}
	
	if(textFile<0){
		printf("Error Opening textFile");
		exit(1);
		
	}
	
	//read in text and keyfiles 
	textFileSize=read(textFile,textBuffer,999);
	n= read(keyFile, keyBuffer,999);
	
	//check keyfile is equal to or longer than textfile 	
	if(n < textFileSize){
		printf("Error, Key file is not long enough\nKeyfile: %d\nText: %d\n",strlen(keyBuffer),strlen(textBuffer) );		
		exit(1);		
	}
		

	//ready socket 
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    
	//connect socket to server 
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
		write(1,"Error: could not contact otp_enc_d on port\n",50 );
		//write("test");
		return(2);		
	} 
        
    //continue looping until we reach end of file. File end is determined when last packet read in from file is less than 999 characters		
	while(fileRead ==0){	
		//send terminating character if below 999 characters
		if(textFileSize <= 999){	
			//send terminating signal
			n = write(sockfd,"@", 1);
			fileRead =1;	
		}
			
		//Send plain textfile  	
		n = write(sockfd,textBuffer,strlen(textBuffer));
		if (n < 0) 
			 error("ERROR writing to socket");
		bzero(textBuffer,1000);
			
		//Read in response, check number off characters matches			
		n = read(sockfd,textBuffer,999); 			
		if (n < 0) 
			 error("ERROR reading from socket");
		if(atoi(textBuffer) < textFileSize){
			printf("otp_enc: Error in sending file. Size of read: %d, Text File Size: %d", strlen(textBuffer), textFileSize);
		}
					
		//send keyfile 	
		n = write(sockfd,keyBuffer,strlen(keyBuffer));
		if (n < 0) 
			 error("ERROR writing to socket");
		//check number of characters sent matches what was recieved
		if(atoi(textBuffer) < textFileSize){
			printf("otp_enc: Error in sending file. Size of read: %d, Text File Size: %d", strlen(textBuffer), textFileSize);
		}
		bzero(keyBuffer,1000);
		//read response
		n = read(sockfd,keyBuffer,999); 			
		//print contents to file
		if(n<1)
			printf("Error Recieving message");
		
				
			
		//print out encrypted results 
		printf("%s", keyBuffer);
		
			
		//check if we have reached end of file,continue to read 
		if(textFileSize >= 999){		
			textFileSize=read(textFile,textBuffer,999);
			n= read(keyFile, keyBuffer,999);
			//check keybuffer and size are still good
			if(strlen(keyBuffer) < strlen(textBuffer)){
				printf("Error, Key file is not long enough\n");
				exit(1);		
			}	
		}
		//done reading files
		else{				
			fileRead =1;
			printf("\n");
		}
	}
    close(sockfd);
    return 0;
}