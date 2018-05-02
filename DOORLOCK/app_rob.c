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
char log_data[32];

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
					
					user_state = 0;
					strcpy( log_data, get_Current_time(user_state) );
				//	insert_LOG(log_data);
				//	system("./tcp_client %s %s", );
					log_socket(log_data);
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
					
					user_state = 1;
					strcpy( log_data, get_Current_time(user_state) );
				//	insert_LOG(log_data);
					log_socket(log_data);
			default : break;
		}
		sleep(2);
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
