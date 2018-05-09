/*
 * Master
 * 
 *  각 IOT 모듈의 제어와 데이터 처리 및 관리.
 *
 */

#ifndef MASTER_H_
#define MASTER_H_

/* Custom libraries. */
#include <server.h>
#include <queue.h>

#include <malloc.h>
#include <pthread.h>
// #include <table_manager.h>

typedef struct master master_t;

struct master {
	server_t			server;		// 서버.
	queue_t				req_queue;	// 요청 큐.
	// table_manager_t table_manager;

	int			on_receive;
	int			on_handle;
};

// 초기화.
int initialize_master(master_t *, const int/*, const char **/);

// 서버 동작.
int run_master(master_t *);

// 요청을 받아 큐에 적재.
void receive_request(master_t *);

// 큐에 있는 요청을 선입 순으로 처리.
void handle_request(master_t *);

#endif
