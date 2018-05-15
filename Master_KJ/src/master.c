/*
 * Master
 * 
 *  각 IOT 모듈의 제어와 데이터 처리 및 관리.
 *
 */
#include <master.h>
#include <stdio.h>
#include <stdlib.h>

/* 초기화 */
int initialize_master(master_t *master, const int port/*, const char *table_path*/)
{
	// 서버 초기화.
	if( initialize_server(&master->server, port) == -1) {
		return -1;
	}

#define QMAX 10
	// 요청큐 초기화.
	if( initialize_queue(&master->req_queue, QMAX) == -1) {
		return -1;
	}

	printf("Initialize success\n");

	/*
	// 장비 테이블 초기화.
	if( initialize_table_manager(&master->table_manager, table_path) == -1 ) { 
	return -1;
	}
	 */

	return 0;
}

/* 동작 */
int run_master(master_t *master)
{
	static pthread_t	receive;	// 요청 받는 스레드.
	static pthread_t	handle; // 요청을 처리하는 스레드.

	// 서버를 동작시킨다.
	// run_server(master->server);

	// 비동기적으로 요청을 받는다.
	pthread_create(
			&receive,
			NULL, 
			(void *(*)(void *))receive_request,
			(void *)master);

	// 비동기적으로 요청을 처리한다.
	pthread_create(
			&handle,
			NULL,
			(void *(*)(void *))respond_cmd,
			(void *)master);

	printf("run successfully\n");
	return 0;
}

/* 요청 적재 루프 */
void receive_request(master_t *master)
{

	printf("receive start.\n");

	master->on_receive = 1;

	while(master->on_receive) {

		printf("receive request\n");

		// 서버로부터 요청을 받는다.
		message_t *request = (message_t *)malloc(sizeof(message_t) );

		receive_message(&master->server, request);

		// 받을 요청을 요청 큐에 적재한다.

		if((doorlock_clnt_sock == request -> clnt_sock) || (esp_clnt_sock == request ->clnt_sock))
		{	
			printf("door : %d , request : %d, esp : %d\n",
				doorlock_clnt_sock, request -> clnt_sock, esp_clnt_sock);
			continue;
		}
		else
		{
			printf("enqueue start\n");
			enqueue(&master->req_queue, (void *)request);
			printf("request->clnt_sock = %d, cmd = %c \n",request->clnt_sock,request->cmd);
			sleep(1);
			printf("enqueue   done\n");
		}


	}
}

/* 요청 처리 루프 */
void handle_request(master_t *master)
{

#ifdef DEBUG
	printf("handle start.\n");
#endif

	master->on_handle = 1;
	while(master->on_handle) {
		// 요청큐에서 요청을 가져온다.
		message_t *request;
		request = dequeue(&master->req_queue);

#ifdef DEBUG
		printf("handle.\n");
#endif
		sleep(1);
		// 요청을 처리한다.
		/***************handle(master, request)*/;
	}
}

/*
   void handle_request(master_t *master)
   {
   master->on_handle = 1;

   while(master->on_handle) {
// 요청 큐가 비어있으면, 대기
if( IS_EMPTY(master->request_queue) ) {
sleep(1);
}

// 요청 큐에서 작업을 하나 가져온다.
message_t *message;
message = dequeue(master->request_queue);

// 요청 대상이 마스터인 경우
if( !message->dev_id) {
// 명령어에 따라 처리.
handle(
message->cmd_code, 
message->data, 
sizeof(message->data);
}
// 요청 대상이 다른 디바이스인 경우
else {
// 디바이스 id로부터 디바이스 ip를 구한다.
char *dev_ip = devid_to_ip(message->dev_id);
// 해당 ip로 명령어 전달.

}

}
}
 */

void respond_cmd(master_t *master)
{
	master->on_handle = 1;

	printf("respond_cmd start\n");

	char cmd;
	int client_sock;
	message_t *request;

	while(master->on_handle) 
	{
		/* 
		 * 요청을 받는다.
		 * 만약 반환된 요청이 없으면(NULL) 계속 있을 때까지 확인한다.
		 */

		request = dequeue(&master->req_queue);
		if(request == NULL) {
			sleep(1);
			continue;
		}

		/* dequeue 함수 사용시 반환할 값이 없으면, NULL 반환한다.
		   이런 경우, if(!request) 또는 if( IS_EMPTY(&master->req_queue) )로 예외처리해야함. */
		
		cmd = request -> cmd;
		client_sock = request->clnt_sock;

		printf("request->clnt_sock  =  %d \n", request->clnt_sock);
		printf("cmd = %c\n",cmd);
		switch(cmd){
			case 'L':   //from Android directory list request
				{
					DIR *dir;
					struct dirent *ent;
					char sendbuf[20];

					dir = opendir("/var/www/html");
					if (dir != NULL){
						while((ent = readdir (dir)) != NULL){
							memset(sendbuf,0x0,sizeof(sendbuf));
							strcpy(sendbuf,ent->d_name);
							printf("%s\n",sendbuf);
							write(client_sock,sendbuf,sizeof(sendbuf));
							memset(sendbuf,0x0,sizeof(sendbuf));
						}
						closedir(dir);
						close(client_sock);
						return ;
					}
					else{
						perror("");
						return ;
					}
				}


			case 'd':   //from detect mass sensor and to rasp2
				{

					int mass_data[1];

					read(client_sock, mass_data, sizeof(mass_data));
					write(doorlock_clnt_sock,mass_data,sizeof(mass_data));

					close(client_sock);
					return ;
				}



			case 'm':   //from Android mood light value and to esp
				{
					char color_data;
					int bright_data;

					read(client_sock, &color_data, sizeof(color_data));
					read(client_sock, &bright_data, sizeof(bright_data));

					write(esp_clnt_sock, &color_data, sizeof(color_data));
					write(esp_clnt_sock, &bright_data, sizeof(bright_data));

					close(client_sock);
					return ;
				}



			case 's':   // receive data and server path
				// rasp2 clit_sock = x
				{	
					FILE *dest;

					char data[128];
					char recv_path_data[30];
					char path[30];

					read(client_sock,recv_path_data,sizeof(recv_path_data));

					strcpy(path,recv_path_data);

					dest = fopen(path,"wb");

					if(dest == NULL)
					{
						perror("File open error");
						exit(0);
					}

					while( recv(client_sock, data, sizeof(data), 0) > 0)
					{
						//fscanf(real_data, "%c", &data);
						fwrite(data, sizeof(data), 1, dest);
					}
					/*
					   while(fscanf( real_data , "%c", &c) > 0) {
					//fprintf(dest, "%c", c);
					}
					 */
					fclose(dest);
					close(client_sock);
					return ;
				}


			case 'u':   //from And unlock cmd and to rasp2
				return ;
			case 't':   //schedule speach?
				return ;
			default:
				printf("Wrong command\n");
		}

	}
}



