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

	return key;
}

char doorlock_get_key(doorlock_t *doorlock)
{
	return ioctl(doorlock->fd, 0, 0);
}

char *get_arr_of_keys(doorlock_t *doorlock, char *keys, char end_key)
{
	int ki;
	char prev_key = end_key;

	for(ki = 0; ki < PASS_SIZE-1; ki++) {
		printf("prev is : %c\n", prev_key);

		while( (keys[ki] = doorlock_get_key(doorlock) ) == '\0'
				 || keys[ki] == prev_key ) {
			if(keys[ki] == '\0') {
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

void unlock(doorlock_t *doorlock)
{
	ioctl(doorlock->fd, 3, 1);
	sleep(5);
	ioctl(doorlock->fd, 3, 0);
}

int play_TTSmp3()
{
	pid_t pid;
	pid = fork();

	if(pid == 0) {
		int ret = system("sh /HomIT/playSound.sh");
		exit(0);
/*	
		if(ret==127 || ret==-1)
			return 0;
		else
			return 1;
			*/
	}
}

char *get_Current_time(int user_state) 
{
	time_t 		timer;
	struct 		tm *t;
	char 		state[10]="";
	static char log_data[32]="";

	if(user_state) 
		    strcpy(state, "going_out "); 
	else 
		    strcpy(state, "visit ");
	//default argc value ~ 1

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
	
	//merge the current time's data
	if( sprintf(log_data, "%s %d %d %d %d %d %d", 
					state, t->tm_year+1900, t->tm_mon+1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec) <0 )
	{
		printf("error - insert_Current_time\n");
		return 0;
	}

	else	
		return log_data;
}
/*
void insert_LOG(char *log_data)
{
	MYSQL       *connection=NULL, conn;
	MYSQL_RES   *sql_result;
	MYSQL_ROW   sql_row;
	int         query_stat;
	char        query[255];

	mysql_init(&conn);

	connection = mysql_real_connect(&conn, DB_HOST,
				DB_USER, DB_PASS,
				DB_NAME, 3306,
				(char *)NULL, 0);

	if (connection == NULL)
	{
		fprintf(stderr, "Mysql connection error : %s", mysql_error(&conn));
		return;
	}

	CHOP(log_data);

	if ( !sprintf(query, "insert into log values ""('%s')", log_data) )
	{
		printf("sprintf error\n");
	}

	query_stat = mysql_query(connection, query);
	if (query_stat != 0)
	{
		fprintf(stderr, "Mysql query error : %s", mysql_error(&conn));
		return;
	}
	else
			printf("inserted LOG data\n");

	mysql_close(connection);
}
*/
int log_socket (char *log_data)
{
	int ssock;
	int clen;
	struct sockaddr_in server_addr;
	char buf[BUFSIZ];

	if((ssock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
			    perror("socket( )");
				    return -1; 
	}

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(TCP_SERVER_addr);
	server_addr.sin_port = htons(TCP_PORT);

	clen = sizeof(server_addr);
	if(connect(ssock, (struct sockaddr *)&server_addr, clen) < 0) {
			    perror("connect( )"); 
				    return -1; 
	}   

	strcpy(buf, log_data);

	if(write(ssock, buf, BUFSIZ) <= 0) {
			    perror("write( )");
				    return -1; 
	}   

	bzero(buf, BUFSIZ);
	if(read(ssock, buf, BUFSIZ) <= 0) {
			    perror("read( )");
				    return -1;
	}   

	printf("Received data : %s \n", buf);

	close(ssock);

	return 0;
}
