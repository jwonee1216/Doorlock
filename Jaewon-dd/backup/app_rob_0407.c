#define _CRT_SECURE_NO_WARNINGS //To skip fopen() warning
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>

int select_menu(void);
static unsigned short flag;
int main(void)
{
	int fd;
	int cont;

	int *readNum;
	int ioctlRtnNum = 0;
	int readRtnNum;

	char passwd[256];
	char realpasswd[256];
	char pwd;

	int  time = 1;
	//	char nowPasswd[10];
	char *nowPasswd;

	FILE *fp;
	int size;
	char *pswdBuf;

	while((cont = select_menu())!= 0){

		switch(cont){
			case '1':
				/*
				   while(1){
				   usleep(250000);
				   printf("pwd : %d\n", ioctl(fd,flag,1));					
				   readNum = ioctl(fd,flag,1);
				   passwd[0] = '\0';
				//memset(passwd, 0, 10);
				if(readNum == 10){
				while(1){
				usleep(250000);
				ioctlRtnNum = ioctl(fd,flag,1);
				if(ioctlRtnNum == 10){
				printf("passwd : %s\n", passwd);
				break;	
				}else{
				if(ioctlRtnNum != 0){
				printf("pwd : %d\n", ioctlRtnNum);
				sprintf(passwd,"%s%d",passwd,ioctlRtnNum);
				}
				}
				}
				break;
				}
				}*/
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
					//LED ON
					ioctl(fd,flag,2);
					usleep(50000);
					while(1){
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

						usleep(100000);
					}

					printf("nowPasswd:%s, passwd:%s,sizenp:%d, sizep:%d\n",pswdBuf,realpasswd,size,strlen(realpasswd));
					if(strcmp(pswdBuf,realpasswd)){
						printf("WRONG PASSWD\n");
						ioctl(fd,flag,3);
						//LED OFF
					}else{
						printf("RIGHT PASSWD\n");
						ioctl(fd,flag,3);
						//unlock door & LED OFF		

					}	
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

