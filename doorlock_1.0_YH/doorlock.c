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
				 || keys[ki] == prev_key ) {
			if(keys[ki] == 147) {
				prev_key = keys[ki];
			}
			usleep(5000);
		}

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

int play_TTSmp3()
{
	int ret = system("sh playSound.sh");
	
	if(ret==127 || ret==-1)
		return 0;
	else
		return 1;
}

int get_Current_time(int user_state) {
	time_t timer;
	struct tm *t;
	char state[10]="";

	if(user_state==1) 
		    strcpy(state, "going_out"); 
	else 
		    strcpy(state, "visit");
	//default argc value ~ 1

	FILE *LOG;
	LOG = fopen("current_Time.txt", "w");

	timer = time(NULL);// 현재 시각을 초 단위로 얻기
	t = localtime(&timer);// 초 단위의 시간을 분리하여 구조체에 넣기

	//printf("Unix Time : %ld Sec\n\n", timer); // 1970년 1월 1일 0시 0분 0초부터 시작하여 현재까지의 초
	printf("STATE\t: %s\n", state);
	printf("YEAR\t: %d\n",   t->tm_year + 1900);
	printf("MONTH\t: %d\n",   t->tm_mon + 1);
	printf("DATE\t: %d\n", t->tm_mday);
	printf("HOUR\t: %d\n",   t->tm_hour);
	printf("MIN\t: %d\n",   t->tm_min);
	printf("SEC\t: %d\n\n", t->tm_sec);
	//printf("현재 요일: %d\n", t->tm_wday); // 일요일=0, 월요일=1, 화요일=2, 수요일=3, 목요일=4, 금요일=5, 토요일=6

	if ( fprintf(LOG, "STATE %s, YEAR %d, MONTH %d, DAY %d, HOUR %d, MINUTE %d, SECOND %d\n", 
							state, t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec) <0 )
			return 0;
			//printf("Error \n");
	else
		    return 1;
			//printf("Stored current_Time.txt\n");
    
	fclose(LOG);
}
