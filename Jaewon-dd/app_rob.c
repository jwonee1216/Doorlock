#include "doorlock.h"

#include <stdio.h>

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>

int main(void)
{
	doorlock_t doorlock;
	char key;
	char keys[PASS_SIZE];
	
	// Initialize.
	if( init_doorlock(&doorlock) < 0) {
#ifdef DEBUG
		printf("error : init_doorlock()\n");
#endif
		return -1;
	}

#ifdef DEBUG
	printf("fd\t\t\t = %d\n", doorlock.fd);
	printf("lock\t\t\t = %s\n", doorlock.lock ? "LOCK" : "UNLOCK");
	printf("password\t\t\t = %s\n", doorlock.pass);
#endif

	// Wait key.
	while(1) {
		key = doorlock_wait_key(&doorlock);
#ifdef DEBUG
		printf("key\t\t\t = %c\n", key);
#endif

		if(key == INPUT_KEY || key == ALTER_KEY) {
			// Get array of keys.
			get_arr_of_keys(&doorlock, keys, key);
			// Match password.
			if( !is_matched_key(&doorlock, keys) ) {
#ifdef DEBUG
				printf("%s - %s\n", doorlock.pass, keys);
				printf("Password is not matched\n");
#endif
				continue;	 
			};
#ifdef DEBUG
			printf("password\t\t\t = %s", doorlock.pass);
#endif 

			if(key == INPUT_KEY) {
				// Unlock
			}
			// key == ALTER_KEY
			else {
				// Get array of keys.
				printf("IN >> \n");
				get_arr_of_keys(&doorlock, keys, key);
				// Alter password.
				alter_password(&doorlock, keys, sizeof(keys) / sizeof(*keys) );
#ifdef DEBUG
				printf("Altered password\t\t\t = %s\n", doorlock.pass);
#endif
			}
		}

		usleep(5000);
	}

	return 0;	
}
