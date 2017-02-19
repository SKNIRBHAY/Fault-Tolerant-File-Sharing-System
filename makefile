CC=gcc
CCOPTS=-g

TARGETS=server_folder/server_session client_folder/myclient_file backup_folder/backup intermediate client2/client

all: $(TARGETS)

server_folder/server_session: server_folder/server_session.c
	$(CC) $(CCOPTS) -o server_folder/server_session server_folder/server_session.c -lpthread -lssl -lcrypto

client_folder/myclient_file: client_folder/myclient_file.c
	$(CC) $(CCOPTS) -o client_folder/myclient_file client_folder/myclient_file.c -lssl -lcrypto

backup_folder/backup: backup_folder/backup.c
	$(CC) $(CCOPTS) -o backup_folder/backup backup_folder/backup.c -lpthread -lssl -lcrypto

client2/client: client2/client.c
	$(CC) $(CCOPTS) -o client2/client client2/client.c -lssl -lcrypto

intermediate: intermediate.c
	$(CC) $(CCOPTS) -o intermediate intermediate.c -lpthread

clean:
	rm -f $(TARGETS) *.o *~
