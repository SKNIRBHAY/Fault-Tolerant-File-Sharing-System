#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/select.h>
#include <pthread.h> 

#define BUFSIZE 1024
#define interport 7777
#define PORTNO_MAIN 4000
#define PORTNO_BACK 5000

void error(char *msg) {
  perror(msg);
  exit(1);
}

struct udp_type
{
	int usock; /*for containing udp socket*/
	struct sockaddr_in client_addr; /*for containing client address*/
	int client_size; /*for containing size of client address*/
	char recvBuff[BUFSIZE]; /*for containing client request*/
};

void *processUdpRequest(void *arg);

int main(int argc, char **argv) 
{

int sockfd; /* socket */
 
  struct sockaddr_in serveraddr_inter;//, serveraddr_M,serveraddr_B; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
 /* client host info */
  //char buf[BUFSIZE]; /* message buf */

  int n; /* message byte size */

   /* 
   * socket: create two sockets of intermediate  --one for MAIN SERVER and one for BACKUP server
   */
sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) 
    error("ERROR opening socket");

    /*
   * build the server's Internet address
   */

  bzero((char *) &serveraddr_inter, sizeof(serveraddr_inter));
  serveraddr_inter.sin_family = AF_INET;
  serveraddr_inter.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr_inter.sin_port = htons(interport);

  /* 
   * bind: associate the intermediate socket (sockfd) with a port number 7777
   */
if (bind(sockfd, (struct sockaddr *) &serveraddr_inter, sizeof(serveraddr_inter)) < 0) error("ERROR on binding socket for client");
while(1)
{
//bzero(buf, BUFSIZE);

//struct udp_type u;
//	u.usock = sockfd;	
//	u.client_size = sizeof(clientaddr);
//n=recvfrom(u.usock,u.recvBuff, sizeof(u.recvBuff)-1, 0,(struct sockaddr *)&u.client_addr, &u.client_size))
   			struct udp_type u;
			u.usock = sockfd;	
			u.client_size = sizeof(struct sockaddr_in);
			memset(u.recvBuff, '0', sizeof(u.recvBuff)); 
	int i;
	i=recvfrom(u.usock,u.recvBuff, sizeof(u.recvBuff), 0,(struct sockaddr *)&u.client_addr, &u.client_size);
	if (i >0)			
	{ printf(" RECEIVED THE MESSAGE FROM CLIENT: %s:", u.recvBuff);
	//u.recvBuff[n] ='\0';
	pthread_t th;
	pthread_create(&th, NULL, processUdpRequest, &u);
	}		
 //n = recvfrom(sockfd, buf, BUFSIZE, 0,(struct sockaddr *) &clientaddr, &clientlen);
    //if (n < 0)
      //error("ERROR in recvfrom");
sleep(1);
} 

close(sockfd);
   return 0;
}

void *processUdpRequest(void *arg)
{

	printf("\n Thread for client");
//struct sockaddr_in clientaddr;
//clientaddr= arg;
//int sockfd;
//sockfd = socket(AF_INET, SOCK_DGRAM, 0);
int sret;  
char buf[BUFSIZE]; /* message buf */
char buf1[BUFSIZE];
char buf2[BUFSIZE];
struct udp_type *tu;
//	char sendBuff[1025];
	tu = (struct udp_type *)arg;

    //memset(sendBuff, '0', sizeof(sendBuff));

	//snprintf(sendBuff, sizeof(sendBuff), "%s",tu->recvBuff);
/*	if( sendto(tu->usock,sendBuff,strlen(sendBuff),0,(struct sockaddr *)&(tu->client_addr),sizeof(tu->client_addr))){
			printf("Response sent\n");
	}*/
//int sockfd = (int)arg;
int sockfd1, sockfd2;
int nsocks=0;
 fd_set checkread;
 struct timeval timeout;
struct sockaddr_in serveraddr_M,serveraddr_B;
  sockfd1 = socket(AF_INET, SOCK_DGRAM, 0);
sockfd2 = socket(AF_INET, SOCK_DGRAM, 0);
//OBJECT FOR MAIN SERVER
	bzero((char *) &serveraddr_M, sizeof(serveraddr_M));
	serveraddr_M.sin_family = AF_INET;
	serveraddr_M.sin_addr.s_addr = inet_addr("127.0.0.1");
	serveraddr_M.sin_port = htons(PORTNO_MAIN);
 	//memset(serveraddr_M.sin_zero, '\0', sizeof serveraddr_M.sin_zero);  
	int serverlen_M = sizeof(serveraddr_M);
	

//OBJECT FOR BACKUP SERVER	
	bzero((char *) &serveraddr_B, sizeof(serveraddr_B));
	serveraddr_B.sin_family = AF_INET;
	serveraddr_B.sin_addr.s_addr = inet_addr("127.0.0.1");
	serveraddr_B.sin_port = htons(PORTNO_BACK);
	int serverlen_B = sizeof(serveraddr_B);
	 //memset(serveraddr_B.sin_zero, '\0', sizeof serveraddr_B.sin_zero);  
        	

	FD_ZERO(&checkread);
	FD_SET(sockfd1,&checkread);
	FD_SET(sockfd2,&checkread);
	
	timeout.tv_sec=5;
	timeout.tv_usec=0;

	strcpy(buf,"file server");

	sendto(sockfd1, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr_M, serverlen_M);
	sendto(sockfd2, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr_B, serverlen_B);
	//if (sockfd1>sockfd2)
	    
	if (sockfd1 > sockfd2)
            nsocks= sockfd1+1;
	else nsocks= sockfd2+1;
        bzero(buf, BUFSIZE);

	sret=select(nsocks,&checkread, NULL,NULL, &timeout); // CHECKING THE MAIN SERVER and Back up server
	
	if (sret== -1)
	{ perror("select");}
        if (sret == 0) {
        printf("\n Timeout occurred!  No data after 5 seconds. | FULL SERVER DOWN");
        } 
	if (sret > 0) 
    	{ printf(" \n *****ANYONE SERVER ACTIVE********");
		  if (FD_ISSET(sockfd1, &checkread)) 
		  {
			printf(" \n SENDING THE MAIN SERVER DETAILS TO CLIENT\n");
			   snprintf(buf1, sizeof(buf1), "%d", PORTNO_MAIN);
			  //sendto(sockfd, buf1, strlen(buf1), 0, (struct sockaddr *) &clientaddr, clientlen);
		 	  if( sendto(tu->usock,buf1,strlen(buf1),0,(struct sockaddr *)&(tu->client_addr),sizeof(tu->client_addr))){
					printf("\nPORTNO sent\n");
				}
		 	  
				
		       	snprintf(buf2, sizeof(buf2), "%s", "127.0.0.1");

		                
			if( sendto(tu->usock,buf2,strlen(buf2),0,(struct sockaddr *)&(tu->client_addr),sizeof(tu->client_addr))){
					//printf("\naddress sent\n");
				}
			 

		        //recvfrom(sockfd1, buf, sizeof(buf), 0, (struct sockaddr *)&serveraddr_M,  &serverlen_M);
		}
	   	else //if(FD_ISSET(sockfd2, &checkread)) 
		{
		printf(" \n SENDING THE BACKUP SERVER DETAILS TO CLIENT\n");

		snprintf(buf1, sizeof(buf1), "%d", PORTNO_BACK);
		  //sendto(sockfd, buf1, strlen(buf1), 0, (struct sockaddr *) &clientaddr, clientlen);
	 	  if( sendto(tu->usock,buf1,strlen(buf1),0,(struct sockaddr *)&(tu->client_addr),sizeof(tu->client_addr))){
				printf("\nPORTNO sent\n");
	 	  }
			
	       	snprintf(buf2, sizeof(buf2), "%s", "127.0.0.1");

	                
		if( sendto(tu->usock,buf2,strlen(buf2),0,(struct sockaddr *)&(tu->client_addr),sizeof(tu->client_addr))){
				printf("\naddress sent\n");
			}
		//recvfrom(sockfd2, buf, strlen(buf), 0, (struct sockaddr *)&serveraddr_B, &serverlen_B);	
		//sendto(sockfd, buf1, sizeof(buf1), 0, (struct sockaddr *) &clientaddr, clientlen);
		//sendto(sockfd, buf2, sizeof(buf2), 0, (struct sockaddr *) &clientaddr, clientlen);
	     }
         
	}
//}
}	
     
