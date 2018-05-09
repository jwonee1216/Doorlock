/*
 * 서버
 * 
 *  간단한 소켓 서버를 구축한다.
 *
 */

#include <server.h>

#ifdef DEBUG
#include <stdio.h>
#endif

int initialize_server(server_t *server, const int port)
{
	// 소켓 생성
	server->socket = socket(PF_INET, SOCK_STREAM, 0);
	if(server->socket == -1) {
		return -1;
	}

	// 서버 주소 길이
	server->lenth_of_address = sizeof(server->address);

	// 서버 주소 초기화
	memset(&server->address, 0, server->lenth_of_address);
	server->address.sin_family		= AF_INET;
	server->address.sin_addr.s_addr	= htonl(INADDR_ANY);
	server->address.sin_port		= htons(port);

	// 서버 소켓에 주소 바인딩(묶음)
	if( bind(
			server->socket, 
			(struct sockaddr *)&server->address, 
			server->lenth_of_address) == -1) {

		return -1;
	}

	return server->socket;
}

void *receive_message(server_t *server, void *buf, size_t size)
{
	int					client_sock;
	socklen_t			client_len; 
	struct sockaddr_in	client_addr;

#define MSGQ_MAX 5
	if( listen(server->socket, MSGQ_MAX) == -1) {
		return NULL;
	}
	
	client_len = sizeof(client_addr);
	client_sock = accept(
		server->socket,
		(struct sockaddr *)&client_addr,
		&client_len);
	if(client_sock == -1) {
		return NULL;
	}


	read(client_sock, (char *)buf, size);
	close(client_sock);
}

/*
int run_server(server_t *server, void *work)
{
	pthread_t thread;
	return pthread_create(&thread, NULL, ( void *(*)(void *) )work, server);
}
*/
