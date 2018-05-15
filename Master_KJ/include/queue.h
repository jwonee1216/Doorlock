#ifndef QUEUE_H_
#define QUEUE_H_

#include <malloc.h>
#include <string.h>

typedef struct message message_t;

struct message {
//	char[20] dev_ip;
	char cmd;
	int clnt_sock;
};

typedef struct queue queue_t;

struct queue {
	message_t **msg;

	int front;
	int rear;

	int max_size;
};

#define IS_MAX(queue)	(queue->front == queue->rear) &&  queue->msg[queue->front]
#define IS_EMPTY(queue)	(queue->front == queue->rear) && !queue->msg[queue->front]

int initialize_queue(queue_t *, const int);

void enqueue(queue_t *, message_t *);
message_t *dequeue(queue_t *);

#endif
