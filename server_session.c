#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h> 
#include <dirent.h> 
#include <pthread.h> 
#include <openssl/des.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>

#define	MAX(x, y)	((x) > (y) ? (x) : (y))
#define PORTNO_MAIN 4000
#define PORTNO_BACKUP 5000
#define IP_MAIN "127.0.0.1"
#define IP_BACKUP "127.0.0.1"

pthread_mutex_t mutex_th;

/*structure to be sent as argument to udp thread*/
struct udp_type{
	int usock; /*for containing udp socket*/
	struct sockaddr_in client_addr; /*for containing client address*/
	int client_size; /*for containing size of client address*/
	char recvBuff[1025]; /*for containing client request*/
};

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

/*This function takes a string str and splits string on basis of delimiter s[] and stores separated strings in **tokens*/
int return_tokens(char *str, char s[], char **tokens){
   char *tok;

   /* get the first token */
   tok = strtok(str, s);

   if(tok == NULL)
        return 0;

   tokens[0] = tok;

   int i = 1;

   while(i < 2) 
   {
      tok = strtok(NULL, s);
      
      if(tok == NULL)
        return 1;

      tokens[i] = tok;
      i++;
   }

   return 2;
}

void *synchronise(void *arg){

	//printf("test in synchronise\n");

	struct sockaddr_in backup_addr;
	char sendBuff[256];
	char recvBuff[256];
	int synfd;
	char s[256];

	char *filename = (char *)arg;
	
	memset(sendBuff, '0',sizeof(sendBuff));

	if((synfd=socket(AF_INET, SOCK_STREAM, 0)) == -1){
		printf("Failed to create socket\n");
		return;
	}

	memset(&backup_addr, '0', sizeof(backup_addr));
	backup_addr.sin_family = AF_INET;
	backup_addr.sin_port = htons(5000);

	if(inet_pton(AF_INET, "127.0.0.1", &backup_addr.sin_addr)== -1){
		printf("Error in reading backup server address\n");
		return;
	}

	//printf("test in synchronise 2\n");

	if(connect(synfd, (struct sockaddr*)&backup_addr, sizeof(backup_addr))== -1){
		printf("Failed to connect\n");
		return;
	}

	//printf("test in synchronise 3\n");

	/*Authentication details for backup server.*/
	int n=read(synfd,recvBuff,sizeof(recvBuff));
		recvBuff[n] = '\0';
		
		if(n){
			printf("Message from Backup server :- %s\n",recvBuff);
			fflush(stdout);
		}

	char authdetails[256];
	strcpy(authdetails,"username");
	strcat(authdetails," ");
	strcat(authdetails,"password\n");
	strcat(authdetails," ");
	snprintf(sendBuff, sizeof(sendBuff),"%s",authdetails);
	write(synfd, sendBuff, strlen(sendBuff));	
	fflush(stdout);

	n=read(synfd,recvBuff,sizeof(recvBuff)-1);
	if(n)
			recvBuff[n] = '\0';


	strcpy(s,"upload ");
	strcat(s,filename);
	strcat(s," ");

	//printf("request for backup server :%s\n",s);

	snprintf(sendBuff, sizeof(sendBuff),"%s",s);

	//printf("request for backup server in sendBuff :%s\n",sendBuff);	
	write(synfd,sendBuff,strlen(sendBuff));
	fflush(stdout);

	sleep(1);/*Giving time to main server to read request*/

	handleDownload(synfd,filename);
}

/*This function opens a file pointer for a file and reads the file to be send to client.*/
int handleUpload(int connfd,char *filename){

	int bytesReceived = 0;
	char recvBuff[256];
	memset(recvBuff, '0',sizeof(recvBuff));
	char filepath[256];

	strcpy(filepath,"./server_repository/"); /*appending root directory path*/
	strcat(filepath,filename);				/*adding filename to make complete file path*/
	
	FILE *fp;

	   //fp = fopen("./client_repository/client_news.txt", "wb"); 
	pthread_mutex_lock (&mutex_th);
		fp = fopen(filepath,"wb");
	    if(NULL == fp)
	    {
	        printf("Error opening file");
	        return -1;
	    }

	    /* Receive data in chunks of 256 bytes */
	    while((bytesReceived = read(connfd, recvBuff, 256)) > 0)
	    {
	        printf("Bytes received %d\n",bytesReceived);  

	        /*Checking decryption*/
	        printf("%s \n", recvBuff);
	        char *decrypted;
            char key[]="passwords";
            decrypted=malloc(sizeof(recvBuff));
            memcpy(decrypted,Decrypt(key,recvBuff,bytesReceived), bytesReceived);
			printf("Decrypted text\t : %s \n",decrypted);
  
	        // recvBuff[n] = 0;
	        //fwrite(recvBuff, 1,bytesReceived,fp);
	        fwrite(decrypted, 1,bytesReceived,fp);
	        fflush(fp);
	        //printf("%s \n", recvBuff);
	    }

	    if(bytesReceived < 0)
	    {
	        printf("\n Read Error \n");
	    }

	    fclose(fp);
		pthread_mutex_unlock (&mutex_th);
		printf("Completed file transfer\n");

	 return 1;

}

/*This function opens a file pointer for a file and reads the file to be send to client.*/
int handleDownload(int connfd,char *filename){

	//printf("in handle download");

	char filepath[256];

	strcpy(filepath,"./server_repository/"); /*appending root directory path*/
	strcat(filepath,filename);	

	 FILE *fp = fopen(filepath,"rb");
        if(fp==NULL)
        {
            printf("File opern error");
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
            char key[]="passwords";
			encrypted=malloc(sizeof(buff));

            memcpy(encrypted,Encrypt(key,buff,sizeof(buff)), sizeof(buff));
			//printf("Encrypted text\t : %s \n",encrypted);  

            if(nread > 0)
            {
                printf("Sending \n");
                ///write(conn_fd, buff, nread);
                write(connfd, encrypted, nread);/*Sending encrypted data over file*/
            }

            /*
             * There is something tricky going on with read .. 
             * Either there was error, or we reached end of file.
             */
            if (nread < 256)
            {
                if (feof(fp))
                    printf("End of file\n");
                if (ferror(fp))
                    printf("Error reading\n");
                break;
            }


        }

        fclose(fp);

        return 1;
}

/*This function lists all the files present in server repository*/
void handlelist(char *s){

	DIR           *d;
	struct dirent *dir;
	 
	strcpy(s,"\n");

	  d = opendir("./server_repository//");
	  if (d)
	  {
	    while ((dir = readdir(d)) != NULL)
	    {
	       if(strcmp(dir->d_name, ".") != 0  && strcmp(dir->d_name, "..") != 0){
	            //printf("%s\n", dir->d_name);
	            strcat(s,dir->d_name);
	            strcat(s,"\n");
	        }
	    }
	    printf("%s\n", s);
	    closedir(d);
	  }

}

char* authorise(char buf[]){
	char s[2] = " ";
   	char *tokens[2];
   	char *users[2]; 
   	FILE *fp;
   	char filebuf[255];

   	int req_params = return_tokens(buf, s, tokens);

   	if(req_params <2){
   		return "Not Authorised";
   	}

   	fp = fopen("user.txt", "r");

   	if(fp == NULL){
   		return "Not Authorisedd";
   	}

   	while(!feof(fp)){
      fgets(filebuf, 255, (FILE*)fp);
      return_tokens(filebuf, s, users);

      if(strcmp(tokens[0],users[0])==0)
      	break;
   }


   	if(strcmp(tokens[0],users[0])==0 && strcmp(tokens[1],users[1])==0)
   		return "Authorised";
   	else 
   		return "Not Authorised";
}

void *checkRequest(void *arg){

	char sendBuff[256];
	char recvBuff[256];
	int n = 0; 
	char s[2] = " ";
   	char *tokens[2];
   	char askreq[256];

   	int conn_fd = (int)arg;

   	memset(recvBuff, '0', sizeof(recvBuff));
   	memset(sendBuff, '0', sizeof(sendBuff));

   	/*Maintaining session start*/
   	strcpy(askreq,"Enter Username and Password");
	snprintf(sendBuff, sizeof(sendBuff), "%s",askreq);

	printf("%s sendbuff\n",sendBuff);
	if(write(conn_fd,sendBuff,strlen(sendBuff))){
				printf("Authentication Required\n");
	}

	if((n=read(conn_fd,recvBuff,sizeof(recvBuff)))>1)
		recvBuff[n] = '\0';
	
	char *auth = authorise(recvBuff);
	printf("auth : %s\n",auth);

	if(strcmp(auth,"Authorised") != 0){
		snprintf(sendBuff, sizeof(sendBuff), "%s",auth);
		if(write(conn_fd,sendBuff,strlen(sendBuff))){
				printf("Authentication Failed Message Sent\n");
				fflush(stdout);
		}
		close(conn_fd);
		return;
	
	}

	/*snprintf(sendBuff, sizeof(sendBuff), "%s",auth);
	if(write(conn_fd,sendBuff,strlen(sendBuff)))
			printf("Authentication Success Message Sent\n");*/
   	
	/*Continuous Loop for serving Client till exits*/
	//while(1){

		strcpy(askreq,"Enter Request for Server.\n1. list\n2.upload filename\n3.download filename\n(to exit type Break)");
		snprintf(sendBuff, sizeof(sendBuff), "%s",askreq);
		//snprintf(sendBuff, sizeof(sendBuff), "Enter Request for Server.\n1. list\n2.upload filename\n3.download filename\n(to exit type Break)");
		if(write(conn_fd,sendBuff,strlen(sendBuff))){
					printf("Authentication Done. Asking for Request \n");
					fflush(stdout);
		}

	   	n=read(conn_fd,recvBuff,sizeof(recvBuff));
			recvBuff[n] = '\0';
			

		printf("Checking request");

		if(n){
				printf("Request from Client :- %s\n",recvBuff);
				fflush(stdout);
		}

		printf("Completed");

		//if(strcmp(recvBuff,"break")==0)
			//break;

	   	int req_params = return_tokens(recvBuff, s, tokens);

		if(req_params == 0)
			printf("Error in req");
			//return "Request parameters invalid";

		else if(req_params == 1 && strcmp(tokens[0],"list")==0){

			char s[1024];
			memset(sendBuff, '0', sizeof(sendBuff)); 
				//printf("testing for list Request from Client :- %s\n",recvBuff);
				handlelist(s);
				snprintf(sendBuff, sizeof(sendBuff), "%s",s);
				if(write(conn_fd,sendBuff,strlen(sendBuff)))
					printf("Response sent\n");

		}else if(req_params == 2 && strcmp(tokens[0],"download")==0){
					int res = handleDownload(conn_fd,tokens[1]);

		}else if(req_params == 2 && strcmp(tokens[0],"upload")==0){
					int res = handleUpload(conn_fd,tokens[1]);
					if(res == 1){
						printf(" result of upload : %d",res);
						/*creating thread to process each client request*/
						pthread_t th;
						pthread_create(&th, NULL, synchronise, (void *)tokens[1]);
					}

		}

		//usleep(100);
	//}

	close(conn_fd);

}


/*This function is called when a new udp client request comes. For new udp client request, a thread is created
and this function is called. The function checks each request and process accordingly for 'list' and 'order'
request.*/
void *processUdpRequest(void *arg){
	struct udp_type *tu;
	char sendBuff[1025];
	tu = (struct udp_type *)arg;

    memset(sendBuff, '0', sizeof(sendBuff));

	snprintf(sendBuff, sizeof(sendBuff), "%s",tu->recvBuff);
	if( sendto(tu->usock,sendBuff,strlen(sendBuff),0,(struct sockaddr *)&(tu->client_addr),sizeof(tu->client_addr))){
			printf("Response sent\n");
	}

}

int main(int argc,char **argv){
	int listen_fd=0, conn_fd = 0;
	int usock;
	struct sockaddr_in my_addr, client_addr;
	int client_size; int port_number;
	char sendBuff[1025];
	char recvBuff[1025];
	int n = 0; 
	int	nfds;
	fd_set	rfds;

	if(argc<1){
		printf("Give proper address of server\n");
		return -1;
	}

	listen_fd = socket(AF_INET, SOCK_STREAM, 0); /*Making File server socket*/

	/*creating socket usock for serving echo clients*/;
	usock = socket(AF_INET, SOCK_DGRAM, 0);
	
	/*Filling values in server address structure*/
	memset(&my_addr, '0', sizeof(my_addr));

	//port_number = atoi(argv[2]);

	my_addr.sin_family = AF_INET;
	//my_addr.sin_port = htons(port_number);
	my_addr.sin_port = htons(4000);

	
	if(inet_pton(AF_INET, "127.0.0.1", &my_addr.sin_addr)== -1){
		printf("error in reading server addrss\n");
		return -1;
	}

	/*Binding socket to particular address */
	int optValue = 1;
	setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&optValue,sizeof(optValue));	

	if(bind(listen_fd,(struct sockaddr*)&my_addr, sizeof(my_addr))==-1){
		printf("Error in binding address to server socket\n");
		return -1;	
	}

	listen(listen_fd,5);


	/*binding socket usock for udp server*/
	setsockopt(usock,SOL_SOCKET,SO_REUSEADDR,&optValue,sizeof(optValue));
	if(bind(usock,(struct sockaddr*)&my_addr, sizeof(my_addr))==-1){
			printf("Error in binding address to udp socket\n");
			return -1;	
	}
	
	client_size = sizeof(struct sockaddr_in);
	printf("server opened for requests\n");


	pthread_mutex_init(&mutex_th, NULL);
	/*Setting arguments for select system call()*/
	nfds = MAX(listen_fd, usock) + 1;	/* bit number of max fd	*/
	FD_ZERO(&rfds);

	/*accept() is inside infinite loop so that server keeps running all the time and accept client request*/
	while(1){

		FD_SET(listen_fd, &rfds);
		FD_SET(usock, &rfds);

		if (select(nfds, &rfds, (fd_set *)0, (fd_set *)0,(struct timeval *)0) < 0){
			printf("Error while calling select() system call");
			return -1;
		}
		 
		if (FD_ISSET(listen_fd, &rfds)) {
			if((conn_fd = accept(listen_fd, (struct sockaddr*)&client_addr,&client_size))==-1){
				printf("Error in connecting to client\n");
				continue;	
			} 

			printf("Server address :- %s : %d\n",inet_ntoa(my_addr.sin_addr),ntohs(my_addr.sin_port));
			printf("Client address :- %s : %d\n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));

			/*creating thread to process each client request*/
			pthread_t th;
			pthread_create(&th, NULL, checkRequest, (void *)conn_fd);
			
			//checkRequest(conn_fd);
		}

		if (FD_ISSET(usock, &rfds)) {
			
			struct udp_type u;
			u.usock = usock;	
			u.client_size = sizeof(struct sockaddr_in);
			memset(u.recvBuff, '0', sizeof(u.recvBuff)); 

			if ((n=recvfrom(u.usock,u.recvBuff, sizeof(u.recvBuff)-1, 0,(struct sockaddr *)&u.client_addr, &u.client_size)) >0){
				u.recvBuff[n] ='\0';
			}
			printf("\nUdp server address    :- %s : %d\n",inet_ntoa(my_addr.sin_addr),ntohs(my_addr.sin_port));
			printf("Udp Client address    :- %s : %d\n",inet_ntoa(u.client_addr.sin_addr),ntohs(u.client_addr.sin_port));	
			printf("Request from udp client :- %s\n",u.recvBuff);

			pthread_t th;
			pthread_create(&th, NULL, processUdpRequest, &u);
		
		}
 
		
		sleep(1);

	}

	close(usock);
	close(listen_fd);
	return 0;

}