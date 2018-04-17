#include "doorlock.h"

int init_doorlock(doorlock_t *doorlock)
{
	FILE *pass_file;

	if(!doorlock) {
		return -1;
	}

	doorlock->fd = open(DL_DD_PATH, O_RDWR);
	if(!doorlock->fd) {
		return -1;
	}

	pass_file = fopen(PASS_PATH, "r");
	if(!pass_file) {
		return -1;
	}
	fscanf(pass_file, "%s", doorlock->pass);
	fclose(pass_file);

	doorlock->lock = 1;

	return doorlock->fd;
}

char doorlock_wait_key(doorlock_t *doorlock)
{
	char key;

	read(doorlock->fd, (char*)&key, sizeof(key) );

	return key + 48;
}

char doorlock_get_key(doorlock_t *doorlock)
{
	char key;

	key = ioctl(doorlock->fd, 0, 1);

	return key + 48;
}

char *get_arr_of_keys(doorlock_t *doorlock, char *keys, char end_key)
{
	int ki;
	char prev_key = end_key;

	for(ki = 0; ki < PASS_SIZE-1; ki++) {
		printf("prev is : %c\n", prev_key);

		while( (keys[ki] = doorlock_get_key(doorlock) ) == 147 
				 || keys[ki] == prev_key ) usleep(5000);

		printf("key is :  %c\n", keys[ki]);
		prev_key = keys[ki];

		if(keys[ki] == end_key) {
			break;
		}
		//sleep(1);
	}
	keys[ki] = '\0';

	return keys;
}

int is_matched_key(doorlock_t *doorlock, char *keys)
{
	return strcmp(doorlock->pass, keys) == 0;
}

void alter_password(doorlock_t *doorlock, char *keys, int size)
{
	FILE *pass_file;

	strcpy(doorlock->pass, keys);

	pass_file = fopen(PASS_PATH, "w");
	if(!pass_file) {
		printf("Error\n");
		return;
	}
	fprintf(pass_file, "%s", keys);
	fclose(pass_file);
}
