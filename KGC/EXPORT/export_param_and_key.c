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
 //TCP/IP FTP(File Transfer Protocol)클라이언트 부분(파라미터 및 키 전송) 
	//디랙토리내 "g.param", "g_1.param", "g_2.param", "h_1.param", "h_2.param", "h_3.param", "h_4.param", "h_5.param","sk_1.key","sk_2.key","sk_3.key","sk_4.key","sk_5.key","sk_6.key"
int main()
{
	int retval;
	int sock;
	int i;
	int count = 1;
	struct sockaddr_in serveraddr;

	char filename[14][256]={"g.param", "g_1.param", "g_2.param", "h_1.param", "h_2.param", "h_3.param", "h_4.param", "h_5.param","sk_1.key","sk_2.key","sk_3.key","sk_4.key","sk_5.key","sk_6.key"};

	FILE *fp1, *fp2, *fp3, *fp4, *fp5, *fp6, *fp7, *fp8, *fp9, *fp10, *fp11, *fp12, *fp13, *fp14;

	if((fp1 = fopen(filename[1], "rb")) != NULL) count++;
	if((fp2 = fopen(filename[2], "rb")) != NULL) count++;
	if((fp3 = fopen(filename[3], "rb")) != NULL) count++;
	if((fp4 = fopen(filename[4], "rb")) != NULL) count++;
	if((fp5 = fopen(filename[5], "rb")) != NULL) count++;
	if((fp6 = fopen(filename[6], "rb")) != NULL) count++;
	if((fp7 = fopen(filename[7], "rb")) != NULL) count++;
	if((fp8 = fopen(filename[8], "rb")) != NULL) count++;
	if((fp9 = fopen(filename[9], "rb")) != NULL) count++;
	if((fp10 = fopen(filename[10], "rb")) != NULL) count++;
	if((fp11 = fopen(filename[11], "rb")) != NULL) count++;
	if((fp12 = fopen(filename[12], "rb")) != NULL) count++;
	if((fp13 = fopen(filename[13], "rb")) != NULL) count++;
	if((fp14 = fopen(filename[14], "rb")) != NULL) count++;

	while 
	// 순차적으로 있는 파마미터 및 키를 모두 전송 
	for (i = 0; i < count; i++)
	{
		sock = socket(PF_INET, SOCK_STREAM, 0);
		if(sock == -1)
			err_quit("socket() error");
		
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_port = htons(9000);
		serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); /*inet_addr("127.0.0.1");*/
		retval = connect(sock, (struct sockaddr*) &serveraddr, sizeof(serveraddr));

		if(retval == -1)
			err_quit("connect() error");

			FILE *fp = fopen(filename[i], "rb");
		if(fp == NULL) 
			err_quit("fopen() error");
	
		printf("filename: %s\n",filename[i]);
		retval = write(sock, filename[i], sizeof(filename[i]));
	
		if(retval == -1)
			err_quit("write() error1");
 
		fseek(fp, 0, SEEK_END);
		int totalbytes = ftell(fp);
 
		retval = write(sock, (char *)&totalbytes, sizeof(totalbytes));
		if(retval == -1)
			err_quit("write() error2");
 
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
		buf[0]='\0';	
	}
	return 0;
}