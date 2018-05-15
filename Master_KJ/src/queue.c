#include <queue.h>

int initialize_queue(queue_t *queue, const int max_size)
{
	queue->msg = (message_t **)malloc( sizeof(message_t*) * max_size);
	if(!queue->msg) {
		return -1;
	}

	memset(queue->msg, 0, sizeof(queue->msg));
	queue->front = queue->rear = 0;
	queue->max_size = max_size;

	return 0;
}

void enqueue(queue_t *queue, message_t *msg)
{
	if( IS_MAX(queue) ) {
		return;	
	}

	queue->msg[queue->rear] = msg;
	queue->rear = (queue->rear + 1) % queue->max_size;
}

message_t *dequeue(queue_t *queue)
{
	message_t *msg;

	if( IS_EMPTY(queue) ) {
		return NULL;
	}

	msg = queue->msg[queue->front];
	queue->msg[queue->front] = NULL;
	queue->front = (queue->front + 1) % queue->max_size;
			
	return msg;
}
