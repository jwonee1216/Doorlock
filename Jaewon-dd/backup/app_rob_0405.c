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
	int *rtn1;
	int rtn2;
	char passwd[10];
	char pwd;
	int time = 1;

	while((cont = select_menu())!= 0){

		switch(cont){
			case '1':
			/*
				while(1){
					usleep(250000);
					printf("pwd : %d\n", ioctl(fd,flag,1));					
					rtn1 = ioctl(fd,flag,1);
					passwd[0] = '\0';
					//memset(passwd, 0, 10);
					if(rtn1 == 10){
						while(1){
							usleep(250000);
							rtn2 = ioctl(fd,flag,1);
							if(rtn2 == 10){
								printf("passwd : %s\n", passwd);
								break;	
							}else{
								if(rtn2 != 0){
									printf("pwd : %d\n", rtn2);
									sprintf(passwd,"%s%d",passwd,rtn2);
								}
							}
						}
						break;
					}
				}*/
				break;
			case '2':

				ioctl(fd,flag,2);

				break; 
			case '3':
				ioctl(fd,flag,3);
				break;
			case '4':
				ioctl(fd,flag,4);
				break;
			case 'r':
				fd = open("/dev/doorlock", O_RDWR);
				printf("fd = %d\n",fd);
				rtn2 = read(fd,rtn1,100);

				printf("read receive data : %d\n",*rtn1);
				printf("read return value : %d\n",rtn2);
				if (fd<0)
				{   
					fprintf(stderr,"/dev/doorlock\n");
					exit(-1);
				}   
				else
					fprintf(stdout,"/dev/doorlock has been detected ... \n");

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

