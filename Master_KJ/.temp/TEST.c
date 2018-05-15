#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <net/server.h>

#define PORT 4040

typedef struct client {
	int					socket;
	struct sockaddr_in	address;	// 목적지 주소
	socklen_t			lenth_of_address;
} client_t;

typedef struct message {
	unsigned char	device_id;
	unsigned char	command_code;
	
	char			*data;
} message_t;

void work(server_t *server)
{
	static client_t		client;		// 클라이언트 중계자
	static message_t	message;	// 전달할 메시지
	static unsigned long data_size;	// 데이터 크기
	static char ip[16];

	while(server) {
		printf("loop\n");
		if( listen(server->socket, 5) == FAIL) {
			continue;	
		}
		
		client.lenth_of_address = sizeof(client.address);

		client.socket = accept(
			server->socket, 
			(struct sockaddr*)&client.address, 
			&client.lenth_of_address);
		
		if (client.socket == FAIL) {
			continue;
		}
		
		read(client.socket, (char *)&message, sizeof(char) * 2);
		
		if(message.command_code == 0x01) {
			data_size = 4;
			printf("command 1!\n");
		}

		if(message.device_id == 0x01) {
			strcpy(ip, "192.168.7.18");
			printf("command 1!\n");
		}

		message.data = (char *)malloc(data_size);
		read(client.socket, message.data, data_size);
		close(client.socket);

		printf("%x/%x/%d\n", message.device_id, message.command_code, *(int*)message.data);


		client.socket  = socket(PF_INET, SOCK_STREAM, 0);
		if(client.socket == FAIL) {
			continue;
			printf("fail\n");
		}

		client.lenth_of_address = sizeof(client.address);
		memset(&client.address, 0, client.lenth_of_address);
		client.address.sin_family     = AF_INET;
		client.address.sin_port       = htons(9090);	// 포트 넘버 나중에 동적으로...
		client.address.sin_addr.s_addr= inet_addr(ip); 

		if( connect( 
			client.socket, 
			(struct sockaddr*)&client.address, 
			client.lenth_of_address) == FAIL) {
			continue;
		}

		int a = 1;
		int b = 255;
		write(client.socket, (char*)&a, sizeof(int) );
		write(client.socket, (char*)message.data, data_size);
		//write(client.socket, &message.command_code, sizeof(char) );
		//write(client.socket, &message.data, data_size);
		free(message.data);
		close(client.socket);
	}
	close(server->socket);
}

int main(int argc, char *argv[])
{
	server_t	server;		// 서버

	// 서버 초기화
	if( FAIL == initialize_server(&server, PORT) ) {
		printf("fail\n");
		return -1;
	}

	// 서버 실행
	run_server(&server, (void *)work);

	while(1)
		sleep(1);

	return 0;
}
