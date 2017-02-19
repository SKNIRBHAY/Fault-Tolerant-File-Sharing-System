#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h> 
#include <openssl/des.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#define BUFSIZE 1024
#define PORTNO 7777

void error(char *msg) {
    perror(msg);
    exit(0);
}

char *Encrypt( char *Key, char *Msg, int size)
{

        static char*    Res;
        int             n=0;
        DES_cblock      Key2;
        DES_key_schedule schedule;

        Res = ( char * ) malloc( size );

        /* Prepare the key for use with DES_cfb64_encrypt */
        memcpy( Key2, Key,8);
        DES_set_odd_parity( &Key2 );
        DES_set_key_checked( &Key2, &schedule );

        /* Encryption occurs here */
        DES_cfb64_encrypt( ( unsigned char * ) Msg, ( unsigned char * ) Res,
                           size, &schedule, &Key2, &n, DES_ENCRYPT );

         return (Res);
}

char *Decrypt( char *Key, char *Msg, int size)
{

        static char*    Res;
        int             n=0;

        DES_cblock      Key2;
        DES_key_schedule schedule;

        Res = ( char * ) malloc( size );

        /* Prepare the key for use with DES_cfb64_encrypt */
        memcpy( Key2, Key,8);
        DES_set_odd_parity( &Key2 );
        DES_set_key_checked( &Key2, &schedule );

        /* Decryption occurs here */
        DES_cfb64_encrypt( ( unsigned char * ) Msg, ( unsigned char * ) Res,
                           size, &schedule, &Key2, &n, DES_DECRYPT );

        return (Res);

}


int uploading(int conn_fd,char *filename){

	//printf("in client upload");

	char filepath[256];

	strcpy(filepath,"./client_repository/"); /*appending root directory path*/
	strcat(filepath,filename);	

	 FILE *fp = fopen(filepath,"rb");
        if(fp==NULL)
        {
            printf("File open error\n");
            return -1;   
        }   

        /* Read data from file and send it */
        while(1)
        {
            /* First read file in chunks of 256 bytes */
            unsigned char buff[256]={0};
            int nread = fread(buff,1,256,fp);
            printf("Bytes read %d \n", nread);   

            /*Encryption for file data*/   
            char *encrypted;
            char *decrypted;
            char key[]="passwords";
			encrypted=malloc(sizeof(buff));
			decrypted=malloc(sizeof(buff));


            memcpy(encrypted,Encrypt(key,buff,sizeof(buff)), sizeof(buff));
			//printf("Encrypted text\t : %s \n",encrypted);  

			//memcpy(decrypted,Decrypt(key,encrypted,sizeof(buff)), sizeof(buff));
			//printf("Decrypted text\t : %s \n",decrypted);

            /* If read was success, send data. */
            if(nread > 0)
            {
                //printf("Sending \n");
                printf("sending.....\n");
                ///write(conn_fd, buff, nread);
                write(conn_fd, encrypted, nread);/*Sending encrypted data over file*/
            }

            if (nread < 256)
            {
                if (feof(fp))
                    printf("End of file\n");
                if (ferror(fp))
                    printf("Error reading\n");
                break;
            }


        }
        printf("\n ***** FILE UPLOADED ***** \n");
        fclose(fp);

        return 1;
}

int downloading(int conn_fd,char *filename){

	int bytesReceived = 0;
	char recvBuff[1025];
	memset(recvBuff, '0',sizeof(recvBuff));
	char filepath[256];

	strcpy(filepath,"./client_repository/"); /*appending root directory path*/
	strcat(filepath,filename);				/*adding filename to make complete file path*/
	
	FILE *fp;

	   //fp = fopen("./client_repository/client_news.txt", "wb"); 

		fp = fopen(filepath,"wb");
	    if(NULL == fp)
	    {
	        printf("Error opening file\n");
	        return 1;
	    }

	    /* Receive data in chunks of 256 bytes */
	    while((bytesReceived = read(conn_fd, recvBuff, 256)) > 0)
	    {
	        //printf("Bytes received %d\n",bytesReceived);    
	        printf("loading.....\n");
	        // recvBuff[n] = 0;

	        /*Checking decryption*/
	        char *decrypted;
            char key[]="passwords";
            decrypted=malloc(sizeof(recvBuff));
            memcpy(decrypted,Decrypt(key,recvBuff,bytesReceived), bytesReceived);
			//printf("Decrypted text\t : %s \n",decrypted);
  
	        // recvBuff[n] = 0;
	        //fwrite(recvBuff, 1,bytesReceived,fp);
	        fwrite(decrypted, 1,bytesReceived,fp);
	        fflush(fp);
	        // printf("%s \n", recvBuff);
	    }

	    if(bytesReceived < 0)
	    {
	        printf("\n Read Error \n");
	    }

	    printf("\n ***** FILE DOWNLOADED ***** \n");
	    fclose(fp);

	 return 0;
}

void listing(int conn_fd){

	char recvBuff[1025];
	memset(recvBuff, '0',sizeof(recvBuff));
	int n = 0;

	n=read(conn_fd,recvBuff,sizeof(recvBuff));
		recvBuff[n] = '\0';
	printf("\n ***** REMOTE FILES *****\n");
	printf("%s\n",recvBuff);

}

/*To display files present in client repository*/
void browsing(){

	DIR           *d;
	struct dirent *dir;
	 
	printf("\n ***** LOCAL FILES *****\n");
	  d = opendir("./client_repository//");
	  if (d)
	  {
	    while ((dir = readdir(d)) != NULL)
	    {
	       if(strcmp(dir->d_name, ".") != 0  && strcmp(dir->d_name, "..") != 0){
	            printf("%s\n", dir->d_name);

	        }
	    }
	    printf("\n");
	    closedir(d);
	  }


}

int main(int argc, char **argv){
	int conn_fd = 0;
	char sendBuff[1025];
	char recvBuff[1025];
	struct sockaddr_in server_addr;
	int i;
	int n = 0;
	int port_number;
	int bytesReceived = 0;
	char s1[256];
	char s2[256];
	char s3[256];


//--------------------------------------UDP to Intermedaite Server-----------------------------------------------------------

	int sockfd;
	int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE] = "message";
	char buf1[BUFSIZE];
	char buf2[BUFSIZE];
       
    	/* socket: create the socket */
    	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    	if (sockfd < 0) 
        	error("ERROR opening socket");

	/* gethostbyname: get the server's DNS entry */
    	server = gethostbyname("localhost");
    	if (server == NULL) {
        	fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        	exit(0);
    	}

    	/* build the server's Internet address */
    	bzero((char *) &serveraddr, sizeof(serveraddr));
    	serveraddr.sin_family = AF_INET;
    	bcopy((char *)server->h_addr, 
		(char *)&serveraddr.sin_addr.s_addr, server->h_length);
    	serveraddr.sin_port = htons(PORTNO);

    	/* send the message to the server */
    	serverlen = sizeof(serveraddr);

    	n = sendto(sockfd, buf, sizeof(buf), 0, (struct sockaddr *) &serveraddr, serverlen);
    	bzero(buf, BUFSIZE);
    	/* print the server's reply 
    	n = recvfrom(sockfd, buf, strlen(buf), 0, (struct sockaddr *) &serveraddr, &serverlen);*/
    	if (n < 0) 
      		error("ERROR in sendto at client");

		n = recvfrom(sockfd, buf1, sizeof(buf1), 0, (struct sockaddr *) &serveraddr, &serverlen);
		if (n < 0){ 
      		error("ERROR in recvfrom");
      		return -1;
      	}
    	//printf("\n PORT NUMBER from server: %s\n", buf1);

		n = recvfrom(sockfd, buf2, sizeof(buf2), 0, (struct sockaddr *) &serveraddr, &serverlen);
		if (n < 0){ 
      		error("ERROR in recvfrom");
      		return -1;
      	}
    	//printf("\n IP ADDRESS from server: %s\n", buf2);

//------------------------------------UDP to Intermediate Server End-------------------------------------------	
	


	if((conn_fd=socket(AF_INET, SOCK_STREAM, 0)) == -1){
		printf("Failed to create socket\n");
		return -1;
	}
	
	memset(&server_addr, '0', sizeof(server_addr));

	port_number = atoi(buf1);

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port_number);

	if(inet_pton(AF_INET, buf2, &server_addr.sin_addr)== -1){
		printf("Error in reading server address\n");
		return -1;
	}
	
	memset(recvBuff, '0',sizeof(recvBuff));

	if(connect(conn_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))== -1){
		printf("Failed to connect\n");
		return -1;
	}
	
	
		n=read(conn_fd,recvBuff,sizeof(recvBuff));
		recvBuff[n] = '\0';
		
		if(n){
			printf("\n ***** CONNECTED *****\n");
			printf("%s\n",recvBuff);
			fflush(stdout);
		}
	
/*shubh_new starts*/
	fgets(sendBuff,sizeof(sendBuff),stdin);
	/*printf("USERNAME :");
	scanf("%s",s1);
	printf("PASSWORD :");
	scanf("%s",s2);
	strcat(s1," ");
	strcat(s1,s2);
	strcat(s1," ");
	printf(" %%%% SHUBH_NEW s1 : %s\n",s1);
	snprintf(sendBuff, sizeof(sendBuff), "%s", s1);	
/*shubhr_new ends*/


	write(conn_fd, sendBuff, strlen(sendBuff));	
	

			n=read(conn_fd,recvBuff,sizeof(recvBuff)-1);
			recvBuff[n] = '\0';
		
			if(n){
				printf("%s\n",recvBuff);
				fflush(stdout);
			}else{
				printf(" ### Connection Problem ###\n");
				return -1;
			}

			if (strcmp(recvBuff,"Not Authorised")==0)
			{
					//printf(" ***** Not Authorised User *****\n");
					close(conn_fd);	
					return -1;
			}

			printf("Enter Request  :");
			scanf("%s",s1);
			if (s1=="break")
				write(conn_fd,s1,strlen(s1));
			else if(strcmp(s1,"list")==0){
				write(conn_fd,s1,strlen(s1));
				listing(conn_fd);

			}

			else{
				//printf("in req else\n");
				if (strcmp(s1,"upload")==0)
					browsing();
				printf("Enter File name :");
				scanf("%s",s2);
				strcpy(s3,s1);
				strcat(s1," ");
				strcat(s1,s2);
				strcat(s1," ");
				snprintf(sendBuff, sizeof(sendBuff), "%s", s1);	
				//printf("sendBuff : %s\n",sendBuff);
				n = write(conn_fd,sendBuff,strlen(sendBuff));	
				if(n <0){
					printf("### Connection Problem ###\n");
					return -1;
				}
				printf("\nprocessing.....\n");
				fflush(stdout);	
				sleep(1);
				
	
				if(strcmp(s3,"download")==0){
					downloading(conn_fd,s2);
				}

				/*if(strcmp(s3,"list")==0){
					listing(conn_fd);
				}*/

				if(strcmp(s3,"upload")==0){
					uploading(conn_fd,s2);
					//printf("resturned from upload\n");
				}

			}

			//usleep(100);
		//}
	


    close(conn_fd);	
    return 0;

 }
