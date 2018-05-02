#ifndef DOORLOCK_H_
#define DOORLOCK_H_

#include <stdio.h>

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define DL_DD_PATH "/dev/doorlock"
#define PASS_PATH "/HomIT/passwd.txt"
#define PASS_SIZE 25

#define INPUT_KEY '*'	// key[*]
#define ALTER_KEY '#'	// key[#]
 
#define INSIDE_BTN 'B'  // inside_btn

#define TCP_PORT 5100
#define TCP_SERVER_addr "192.168.7.12"

struct doorlock {
	int fd;

	int lock;
	char pass[PASS_SIZE];
};

typedef struct doorlock doorlock_t;

int init_doorlock(doorlock_t *);

char doorlock_wait_key(doorlock_t *);

char *get_arr_of_keys(doorlock_t *, char *, char);

int is_matched_key(doorlock_t *, char *);

void alter_password(doorlock_t *, char *, int);

void unlock(doorlock_t *);

int play_TTSmp3();

char *get_Current_time(int);

//void insert_LOG(char *logdata);

int log_socket(char *);

#endif // !DOORLOCK_H_
