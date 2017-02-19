# Fault-Tolerant-File-Sharing-System
This project is developed in Ubuntu environment by installing VM on Windows 10.
Ubuntu version : 14.04 LTS

The project contains 7 files:-
1.  server_session.c :- Handles client file access requests. 
2.  backup.c :- Backup to main server.
3.  intermediate.c :- Monitors status of backup and main server. Provides client with active server's IP Address and port number.
4.  myclient_file.c :- File client that requests server for file list/upload/download.
5.  client.c :- Clone to file client. Can be run to display concurrency.
5.  makefile :- compiles all the above .c files and create executable file. It also removes all output files.
7.  Readme   :- contains information of all files included in project and steps to use different files.

*NOTE :- There are seaparate repositories for main and backup server and each client.

Steps to run.

1) Open Terminal

2) Locate to directory on terminal where all project files are kept

3) Enter command 'make' on terminal - it will create the executables as
   intermediate
   server_folder/server_session
   backup_folder/backup
   client_folder/myclient_file  
   client2/client
 
4) Run all servers : 
	i) Run intermediate server
	- ./intermediate

	ii) Run main server :- Open separate terminal
	- cd server_folder 
	- ./server_session

	ii) Run backup server :- Open separate terminal
	- cd backup_folder 
	- ./backup

4) Run client : 
	i) Open separate terminal
	- cd client_folder 
	- ./myclient_file

	*Note - Client will be prompted to enter username and password. After authenticated, client can make various requests to access file to server.

7) To remove all output files, enter command 
			- make clean


** PLEASE CHECK DEMO VIDEO (Video_for_project_Fault_Tolerant_File_Sharing_System) FOR BETTER UNDERSTANDING OF THE PROJECT WORKING **

References :- 
1.	 https://en.wikipedia.org/wiki/Clustered_file_system#Distributed_file_systems 

2.	 http://sce.uhcl.edu/helm/rationalunifiedprocess/process/workflow/ana_desi/co_cncry.  htm 
3.	 http://www.cs.dartmouth.edu/~campbell/cs50/socketprogramming.html 

4.	 http://beej.us/guide/bgnet/output/print/bgnet_A4.pdf 

5.	 http://www.highteck.net/EN/Application/Application_Layer_Functionality_and_Protocols.html 

6.	 http://www.01.ibm.com/support/docview.wss?uid=nas8N1013578
