/*
 * 서버
 * 
 *  간단한 소켓 서버를 구축한다.
 *
 */

#ifndef NET_SERVER_H
#define NET_SERVER_H

#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <arpa/inet.h>

#include <pthread.h>

typedef struct server server_t;

struct server {
	int					socket;				// 서버 소켓.	
	socklen_t			lenth_of_address;	// 주소 길이.
	struct sockaddr_in	address;			// 네트워크 주소.
};

// 초기화.
int initialize_server(server_t *, const int);
void *receive_message(server_t *, void *);

int doorlock_clnt_sock;
int esp_clnt_sock;
/*
// 서버 동작.
int run_server(server_t *, void *);
*/

#endif // !NET_SERVER_H
