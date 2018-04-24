#include "doorlock.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

doorlock_t doorlock;
char key;
char keys[PASS_SIZE];
int user_state;
	
int check(void);
void alter(void);

int main(void)
{
	// 초기 설정
	init_doorlock( &doorlock );
#ifdef DEBUG
	printf("password = %s\n", doorlock.pass);
#endif

	while(1) {
		// 대기 모드
		key = doorlock_wait_key(&doorlock);
#ifdef DEBUG
		printf("irq key = %c\n", key);
#endif
		switch(key) {
			// '*' 일 경우,
			case '*': 
				// 비밀번호 확인
				if( check() ) {
					play_TTSmp3();	
					unlock(&doorlock);	
					//user_state = 0;
					//get_Current_time(user_state);
				}
				break;
			case '#': 
				if( check() ) {
					alter();
				}
				break;
			case 'B': 
					play_TTSmp3();
					unlock(&doorlock);
					//user_state = 1;
					//get_Current_time(user_state);
			default : break;
		}
		sleep(2);

		//key = doorlock_wait_key(&doorlock);
/*
		switch(key) {
			case '*': 
			case '#': 
			case 'B': 
				if( !play_TTSmp3() ) {
					return -1;
				}

				unlock(&doorlock);

				user_state = 1;	
				if( !get_Current_time(user_state) ) {
					return -1;
				}
				break;
			default : break;
		}
		sleep(2);
		// 만약 # 버튼
		// 만약 UNLOCK 버튼
		if(key == INSIDE_BTN) {
			//play TTS mp3file
			if(!play_TTSmp3())
			    printf("failed playing Sound\n");
			else
			    printf("played weather_notice.mp3\n");
			//Unlock
			unlock(&doorlock);

			//get current time
			user_state = 1;
			//going out
			if(!get_Current_time(user_state))
					printf("error - get_current_time Fuction");
			else
					printf("stored Current time Text file");
		}
		

		if(key == INPUT_KEY || key == ALTER_KEY) {
			ioctl(doorlock.fd, 1, 1);
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

			switch (key) {
				case INPUT_KEY :
					unlock(&doorlock);	
					user_state = 0;
				// visit
				if(!get_Current_time(user_state))
						printf("error - get_current_time Fuction");
				else
						printf("stored Current time Text file");
					break;

				case ALTER_KEY :
					// Get array of keys.
					printf("IN >> \n");
					memset(keys, 0, sizeof(keys)); 
					get_arr_of_keys(&doorlock, keys, key);
					// Alter password.
					alter_password(&doorlock, keys, sizeof(keys) / sizeof(*keys) );
					break;
				default : break;
			}
			ioctl(doorlock.fd, 1, 0);
		}
*/
	}
	return 0;	
}	

int check(void) {
	ioctl(doorlock.fd, 1, 1);
#ifdef DEBUG
	printf("LED ON\n");
#endif

	memset(keys, 0, sizeof(keys) );
	get_arr_of_keys(&doorlock, keys, key);

#ifdef DEBUG
	printf("password = %s\n", doorlock.pass);
	printf("input = %s\n", keys);
	printf("LED OFF\n");
#endif
	ioctl(doorlock.fd, 1, 0);

	return is_matched_key(&doorlock, keys);
}

void alter(void) {
	memset(keys, 0, sizeof(keys)); 
	get_arr_of_keys(&doorlock, keys, key);
	alter_password(&doorlock, keys, sizeof(keys) / sizeof(*keys) );
}
