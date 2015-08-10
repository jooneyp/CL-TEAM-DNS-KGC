#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
 
#define BUFSIZE 4096
 
void err_quit(char *msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
	}

 //TCP/IP FTP(File Transfer Protocol)클라이언트 부분(ID 전송) 
	//디랙토리내 "Input_Domain_name.txt"
int main()
{
	int retval;
	int sock;
	struct sockaddr_in serveraddr;
 
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if(sock == -1)
		err_quit("socket() error");
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(9000);
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); /*inet_addr("127.0.0.1");*/
	retval = connect(sock, (struct sockaddr*) &serveraddr, sizeof(serveraddr));
	if(retval == -1)
		err_quit("connect() error");
 

	FILE *fp = fopen("Input_Domain_name.txt", "rb");
	if(fp == NULL) 
		err_quit("fopen() error");
 
	char filename[256];
	memset(filename, 0, sizeof(filename));
	strcpy(filename, "Input_Domain_name.txt");
	printf("%s",filename);
	
	retval = write(sock, filename, sizeof(filename));
	if(retval == -1)
		err_quit("write() error");
 
	fseek(fp, 0, SEEK_END);
	int totalbytes = ftell(fp);
 
	retval = write(sock, (char *)&totalbytes, sizeof(totalbytes));
	if(retval == -1)
		err_quit("write() error");
 
	char buf[BUFSIZE];
	int numread;
	int numtotal = 0;
 
	rewind(fp);
	while(1) {
		numread = fread(buf, 1, BUFSIZE, fp);
		if(numread > 0) {
			retval = write(sock, buf, numread);
			if(retval == -1)
				err_quit("write() error!");
			numtotal += numread;
		}
		else if(numread == 0 && numtotal == totalbytes) {
			printf("file trans complete : %d bytes\n", numtotal);
			break;
		}
		else {
			err_quit("file I/O error");
		}
	}
	fclose(fp);
	close(sock);
 
	return 0;
}