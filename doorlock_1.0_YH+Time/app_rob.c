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
		return -1;
	}

	printf("fd\t\t\t = %d\n", doorlock.fd);
	printf("lock\t\t\t = %s\n", doorlock.lock ? "LOCK" : "UNLOCK");
	printf("password\t\t\t = %s\n", doorlock.pass);

	// Wait key.
	while(1) {
		int user_state;
		//going out -> reset to 1
		//visit -> reset to 0

		key = doorlock_wait_key(&doorlock);
		printf("key\t\t\t = %c\n", key);
		if(key == INSIDE_BTN) {
			//Unlock

			//play TTS mp3file
			if(!play_TTSmp3)
			    printf("failed playing Sound\n");
			else
			    printf("played weather_notice.mp3\n");
			
			//get current time
			user_state = 1;
			//going out
			if(!get_Current_time(user_state))
					printf("error - get_current_time Fuction");
			else
					printf("stored Current time Text file");
			continue;
		}

		if(key == INPUT_KEY || key == ALTER_KEY) {
			// Get array of keys.
			memset(keys, 0, sizeof(keys));
			get_arr_of_keys(&doorlock, keys, key);
			// Match password.
			//getchar();
			if( !is_matched_key(&doorlock, keys) ) {
				printf("%s - %s\n", doorlock.pass, keys);
				printf("Password is not matched\n");
				//sleep(2);
				key = -1;
			}
			else {
				printf("%s - %s\n", doorlock.pass, keys);
				printf("Password is matched\n");
				//sleep(2);
			}

			if(key == INPUT_KEY) {
				// Unlock
			}
			if(key == ALTER_KEY) {
				// Get array of keys.
				printf("IN >> \n");
				memset(keys, 0, sizeof(keys));
				get_arr_of_keys(&doorlock, keys, key);
				// Alter password.
				alter_password(&doorlock, keys, sizeof(keys) / sizeof(*keys) );
			}
		}

		sleep(2);
	}

	return 0;	
}
