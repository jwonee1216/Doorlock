#define _CRT_SECURE_NO_WARNINGS //To skip fopen() warning
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>

int serv_sock;
int clnt_sock;

void error_handling(char *message);
void signalHandler(int signo){
	write(clnt_sock,"quit",4);
	close(clnt_sock);
	close(serv_sock);
	printf("plz one more ctrl + c\n");
	signal(SIGINT,SIG_DFL);
}

void error_handling(char* message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

int select_menu(void);
static unsigned short flag;
int main(void)
{
	int fd;
	int cont;

	int *readNum;
	int ioctlRtnNum = 0;
	int readRtnNum;

	char passwd[1024];
	char realpasswd[256];
	char pwd;

	int  time = 1;
	//	char nowPasswd[10];
	char *nowPasswd;

	FILE *fp;
	int size;
	char *pswdBuf;

	//chat S
	int msg_len;
	int str_len;
	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	int clnt_addr_size;
	char message[] = "Hello World!\n";
	char chatMsg[128];
	//chat E


	while((cont = select_menu())!= 0){

		switch(cont){
			case '4':
				//unlock 
				fd = open("/dev/doorlock", O_RDWR);
				printf("fd =%d\n",fd);
				ioctlRtnNum = ioctl(fd,flag,4);
				printf("ioctl rtn:%d\n", ioctlRtnNum);
				close(fd);
				break;
			case '5':
				//lock 
				fd = open("/dev/doorlock", O_RDWR);
				printf("fd =%d\n",fd);
				ioctlRtnNum = ioctl(fd,flag,5);
				printf("ioctl rtn:%d\n", ioctlRtnNum);
				close(fd);
				break;
			case '6':
				//lock 
				fd = open("/dev/doorlock", O_RDWR);
				printf("fd =%d\n",fd);
				ioctlRtnNum = ioctl(fd,flag,5);
				printf("ioctl rtn:%d\n", ioctlRtnNum);
				close(fd);
				break;
			case '7':
				/*서버 소켓 생성*/
				serv_sock = socket(PF_INET, SOCK_STREAM, 0);
				if(serv_sock == -1)
				{
					error_handling("socket() error");
				}
				memset(&serv_addr, 0, sizeof(serv_addr));
				serv_addr.sin_family = AF_INET;
				serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
				serv_addr.sin_port = htons(8282);

				/*소켓에 주소 할당*/
				if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
				{
					error_handling("bind() error");
				}

				while(1){
					/*연결 요청 대기상태로 진입*/
					if(listen(serv_sock, 5) == -1)
					{
						error_handling("listen() error");
					}
					/*연결 요청 수락*/
					clnt_addr_size = sizeof(clnt_addr);
					clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
					if(clnt_sock == -1)
					{
						error_handling("accept() error");
					}

					//signal(SIGINT, signalHandler);
					while(1){
						printf("SERVER:");
						gets(chatMsg);
						msg_len=strlen(chatMsg);
						/*데이터 전송*/
						write(clnt_sock, chatMsg, msg_len);
						if(strcmp(chatMsg,"quit")==0){
							break;
						}

						str_len= read(clnt_sock, chatMsg, sizeof(chatMsg)-1);
						message[str_len]=0;
						printf("client:%s\n", chatMsg);

					}
					/*연결 종료*/
					close(clnt_sock);
				}
			break;

			case 'r':
				//To get present passwd 
				fp = fopen("/passwd.txt","r");
				fseek(fp, 0, SEEK_END);
				size = ftell(fp);
				pswdBuf = (char*)malloc(size);
				memset(pswdBuf,0,size);
				fseek(fp,0,SEEK_SET);
				fread(pswdBuf,size-1,1,fp);
				printf("nowpasswd:%s,filesize:%d\n",pswdBuf,size);

				fd = open("/dev/doorlock", O_RDWR);
				printf("fd = %d\n",fd);

				readNum = (int*)malloc(sizeof(int));
				readRtnNum = read(fd,readNum,100);

				printf("read receive data : %d\n",*readNum);
				printf("read return value : %d\n",readRtnNum);

				passwd[0] = '\0';
				realpasswd[0] = '\0';
				if(*readNum == 10){
					//outside unlock
					//LED ON
					usleep(50000); //To make term otherwise right finished
					ioctl(fd,flag,2);
					while(1){
						usleep(100000);
						ioctlRtnNum = ioctl(fd,flag,1);
						int i;
						char temp = 'A';

						if(ioctlRtnNum == 10 || ioctlRtnNum == 12){
							printf("passwd:%s\n", passwd);
							for(i = 0 ; i < strlen(passwd); i++){
								if(passwd[i] != 'A'){
									if(temp != passwd[i]){
										printf("temp:%c, passwd:%c\n",temp,passwd[i] );
										sprintf(realpasswd,"%s%c",realpasswd,passwd[i]);
									}
								}
								temp = passwd[i];
							}	
							break;	
						}else{
							if(ioctlRtnNum == 99){
								printf("pwd:%d\n", ioctlRtnNum);
								sprintf(passwd,"%s%c",passwd,'A');
							}else{
								printf("pwd:%d\n", ioctlRtnNum);
								sprintf(passwd,"%s%d",passwd,ioctlRtnNum);
							}


						}

					}

					printf("nowPasswd:%s, passwd:%s,sizenp:%d, sizep:%d\n",pswdBuf,realpasswd,size,strlen(realpasswd));
					if(strcmp(pswdBuf,realpasswd)){
						printf("WRONG PASSWD\n");
						ioctl(fd,flag,3);
						//LED OFF
					}else{
						printf("RIGHT PASSWD\n");
						ioctl(fd,flag,3); //LED OFF
						//ioctl(fd,flag,4); //unlock
					}	
				}else if(*readNum == 12){
					//changigng password by JW

				}else if(*readNum == 13){
					//unlock & TTS by pressing inside btn by YH

				}	


				if (fd<0)
				{   
					fprintf(stderr,"/dev/doorlock\n");
					exit(-1);
				}   
				else
					fprintf(stdout,"/dev/doorlock has been detected ... \n");

				free(pswdBuf);
				free(readNum);

				fclose(fp);
				close(fd);	

				break;

			case 'o':
				fd=open("/dev/doorlock",O_RDWR);	
				printf("fd=%d\n",fd);
				break;
			case 'c':
				close(fd);
				break;
			case 'q':
				printf("exit\n");
				exit(-1);
				break;
			default:
				break;

		}

	}
	return 0;
}


int select_menu(void)
{
	int key;
	printf("\n");
	printf("****** SELECT INPUT MENU *********\n");
	printf("** 1. ROB --> IOCTL 1 **\n");
	printf("** 2. ROB --> IOCTL 2 **\n");
	printf("** 3. ROB --> IOCTL 3 **\n");
	printf("** 4. ROB --> IOCTL 4 **\n");
	printf("** 4. ROB --> IOCTL 5 **\n");
	printf("** 4. ROB --> IOCTL 7 **\n");
	printf("** **\n");
	printf("** o. ROB --> OPEN **\n");
	printf("** c. ROB --> CLOSE **\n");
	printf("** r. ROB --> OPEN & READ  **\n");
	printf("** **\n");
	printf("** q. ROB --> QUIT **\n");
	printf("=================================\n");
	printf("\n\n");
	printf("select the command number : ");
	key = getchar();
	getchar();
	fflush(stdin);
	return key;
}

