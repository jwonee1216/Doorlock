#ifndef CLIENT
#define CLIENT

struct client;

#define CLIENT struct client

#endif // !CLIENT_H_

typedef struct client client_t;

struct client {
	int sock;
	struct sockaddr_in addr;
	int addr_size;
} client_t;

void inicli(client_t *cli)
{
	cli->sock = 
}


void run_server(server_t *server)
{
	client_t client;

	while(server) {
		
		printf(".");
		if(listen(server->sock, MAX_CLI_NUM) == -1) {
			continue;
		}
		
		client.sock = accept(server->sock, (struct sockaddr *)&client.addr, &client.addr_size);

		if(client.sock == -1) {
			continue;
		}

		message_t message;
		read(client.sock, (char *)&message, sizeof(char) * 2);

	}
	close(client.sock);
}

