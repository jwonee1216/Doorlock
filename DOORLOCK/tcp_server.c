#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <string.h>

#define TCP_PORT  5100

int add_logTxt (char *buf)
{
	FILE *pFile = NULL;

	pFile = fopen("current_Time.txt", "a+");
	if( fprintf(pFile, "%s\n", buf) <0)
	{
	    printf("error\n");
		return -1;
	}
	else 
	{
	    printf("Stored current_Time.txt\n");
	    fclose(pFile);
		return 0;
	}

}

int main(int argc, char** argv)
{
	int ssock;
	socklen_t clen;
	int n;
	struct sockaddr_in client_addr, server_addr;
	char buf[BUFSIZ];

	//FILE *pFile = NULL;

	if((ssock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket( )");
		return -1; 
	}  

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(TCP_PORT);

	if(bind(ssock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		perror("bind( )");
		return -1; 
	}   

	if(listen(ssock, 8) < 0) {
		perror("listen( )");
		return -1; 
	}   

	clen = sizeof(client_addr);

	while(1) {
		int csock = accept(ssock, (struct sockaddr *)&client_addr, &clen);

		printf("Client is connected : %s\n", inet_ntoa(client_addr.sin_addr));

		if((n = read(csock, buf, BUFSIZ)) <= 0)
			perror("read( )");

		printf("Received data : %s\n", buf);
/**
		//ADD Insert the buf to txtfile
		pFile = fopen("current_Time.txt", "a+");
		if( fprintf(pFile, "%s\n", buf) <0)
		{
			printf("error\n");
		}
		else 
		{
			printf("Stored current_Time.txt\n");
			fclose(pFile);
		}
*/
		add_logTxt(buf);
		if(write(csock, buf, n) <= 0)
			perror("write( )");

		close(csock);
	}   

	close(ssock);             /* 서버 소켓을 닫음 */

	return 0;
}

