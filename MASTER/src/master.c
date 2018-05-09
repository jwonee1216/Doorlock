/*
 * Master
 * 
 *  각 IOT 모듈의 제어와 데이터 처리 및 관리.
 *
 */
#include <master.h>

#ifdef DEBUG
#include <stdio.h>
#endif

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

#ifdef DEBUG
	printf("Initialize success\n");
#endif

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
		(void *(*)(void *))handle_request,
		(void *)master);

#ifdef DEBUG
	printf("run successfully\n");
#endif

	return 0;
}

/* 요청 적재 루프 */
void receive_request(master_t *master)
{

#ifdef DEBUG
	printf("receive start.\n");
#endif

	master->on_receive = 1;
	while(master->on_receive) {
		// 서버로부터 요청을 받는다.
		message_t *request = (message_t *)malloc(sizeof(message_t) );
		receive_message(&master->server, request, sizeof(*request) );

#ifdef DEBUG
	printf("MSG : %x/%x/%d\n", request->dev, request->cmd, request->data_size);
#endif 

		// 받을 요청을 요청 큐에 적재한다.
		enqueue(&master->req_queue, (void *)&request);
		sleep(1);

#ifdef DEBUG
	printf("receive.\n");
#endif

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
