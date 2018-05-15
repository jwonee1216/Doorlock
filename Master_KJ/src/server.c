/*
 * 서버
 * 
 *  간단한 소켓 서버를 구축한다.
 *
 */

#include <stdio.h>
#include <server.h>
#include <queue.h>



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

void *receive_message(server_t *server, void *request)
{

	

	int					client_sock;
	socklen_t			client_len; 
	struct sockaddr_in	client_addr;
	message_t *req;
	req = (message_t *)request;

	doorlock_clnt_sock = -1;
	esp_clnt_sock = -1;



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

	/*      
			ROB S  
	 */ 

	// IPv4 demo of inet_ntop() and inet_pton()

	char clnt_ip[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &(client_addr.sin_addr), clnt_ip, INET_ADDRSTRLEN);
	printf("%s\n", clnt_ip); // prints "192.0.2.33"
	

	if(strcmp(clnt_ip,"192.168.7.11") == 0){
		doorlock_clnt_sock = client_sock;
		req->clnt_sock = client_sock;

	}
	else if (strcmp(clnt_ip,"esp_wifi_ip") == 0)
			{
				esp_clnt_sock = client_sock;
				req->clnt_sock = client_sock;
			}
	else{

	req->clnt_sock = client_sock;
	printf("server accepted client sock = %d\n",client_sock);
	printf("req->clnt_sock = %d\n",req->clnt_sock);
	read(client_sock, &req->cmd, sizeof(char));
	printf("server received cmd = %c\n",req->cmd);
	
	//close(client_sock);
			}
	/*
	   ROB E
	 */     
}



			/*
			   int run_server(server_t *server, void *work)
			   {
			   pthread_t thread;
			   return pthread_create(&thread, NULL, ( void *(*)(void *) )work, server);
			   }
			 */
