#ifndef DOORLOCK_H_
#define DOORLOCK_H_

#include <stdio.h>

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>

#define DL_DD_PATH "/dev/doorlock"
#define PASS_PATH "/HomIT/passwd.txt"
#define PASS_SIZE 25

#define INPUT_KEY '*'	// key[*]
#define ALTER_KEY '#'	// key[#]
 
#define INSIDE_BTN 'B'  // inside_btn


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

int get_Current_time(int move);
#endif // !DOORLOCK_H_
