// DNS Daemon
// 1. 데몬형태로 소켓을 열고 돌고있어야 한다.
// 2. Setup을 이용하여 Key를 가지고있어야 함.
// 2. 소켓으로 URL / IP가 들어옴.
// 3. URL을 이용하여 Keygen 실행. (new_key_level_1)
// 4. My_param 8개 키와 앞서 생성된 키 (총 9개)를 상대에게 보냄.
// 5. /etc/named.rfc~.conf 최하단에 설정내용 추가.
// 6. /var/named/<url>.conf 파일 추가 후 설정내용 삽입.
// 7. named restart

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "BB_Setup_Keygen.h"

#define BUF_SIZE 4096
#define MAX_CLNT 256

void * handle_clnt(void * arg);
void send_param();
void error_handling(char * msg);
void open_socket();

int clnt_cnt=0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;
char clnt_IP[20];

int main(int argc, char **argv) {
	BB_SYS_PARAM bb_param;
	int param_opt;

	while (-1 != (param_opt = getopt(argc, argv, "sd"))) {
		switch(param_opt)
		{
			case 's' :
				printf("BB_Setup---------------------------------------\n");
				BB_Setup(&bb_param);
				printf("BB_Setup---------------------------------------\n");
				break;
			case 'd' :
				printf("BB KGC Daemon Start\n");
				open_socket();
				// 소켓열고 대기
				// URL / IP 들어오면 Keygen 실행
				// 만들어진 키를 전송
				break;
			case '?' :
				printf("Usage : %s <mode (sd)> [option]\n", argv[0]);
				return 0;
		}
	}

	return 0;
}

void open_socket() {
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	pthread_t t_id;
	pthread_mutex_init(&mutx, NULL);
	serv_sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET; 
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(5959);
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");
	if(listen(serv_sock, 5) == -1)
		error_handling("listen() error");
	
	while(1)
	{
		clnt_adr_sz=sizeof(clnt_adr);
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);
		
		pthread_mutex_lock(&mutx);
		clnt_socks[clnt_cnt++] = clnt_sock;
		pthread_mutex_unlock(&mutx);
	
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock);
		pthread_detach(t_id);
		printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr));
		strcpy(clnt_IP, inet_ntoa(clnt_adr.sin_addr));
	}
	close(serv_sock);
}

void * handle_clnt(void * arg)
{
	BB_SYS_PARAM bb_param;
	FILE *fp;
	char filename[14][256] = {
		"g.param", "g_1.param", "g_2.param", "h_1.param", "h_2.param", "h_3.param", "h_4.param", "h_5.param","sk_1.key","sk_2.key","sk_3.key","sk_4.key","sk_5.key","sk_6.key"
	};
	int clnt_sock = *((int*)arg);
	int i=0;
	int retval;
	char URL[BUF_SIZE];
	char *IP;
	
	read(clnt_sock, URL, sizeof(URL)); // 들어오는 URL과 IP를 URL에 받는다.
	IP = strtok(URL, "^");
	BB_Keygen(URL, &bb_param);
	while ((fp = fopen(filename[i], "rb")) != NULL ) {
		retval = write(clnt_sock, filename[i], sizeof(filename[i]));
	
		if(retval == -1)
			error_handling("write() error1");
 
		fseek(fp, 0, SEEK_END);
		int totalbytes = ftell(fp);
 
		retval = write(clnt_sock, (char *)&totalbytes, sizeof(totalbytes));
		if(retval == -1)
			error_handling("write() error2");
 
		char buf[BUF_SIZE];
		int numread;
		int numtotal = 0;
 
		rewind(fp);
		while(1) {
			numread = fread(buf, 1, BUF_SIZE, fp);
			if(numread > 0) {
				retval = write(clnt_sock, buf, numread);
				if(retval == -1)
					error_handling("write() error!");
				numtotal += numread;
			}
			else if(numread == 0 && numtotal == totalbytes) {
				printf("file trans complete : %d bytes\n", numtotal);
				break;
			}
			else {
				error_handling("file I/O error");
			}
		}
		fclose(fp);
		i++;
	}
	pthread_mutex_lock(&mutx);
	for(i=0; i<clnt_cnt; i++)   // remove disconnected client
	{
		if(clnt_sock==clnt_socks[i])
		{
			while(i++<clnt_cnt-1)
				clnt_socks[i]=clnt_socks[i+1];
			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx);
	close(clnt_sock);
	return NULL;
}

void send_param()   // send to all
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
		serveraddr.sin_port = htons(5959);
		serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
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

void error_handling(char * msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}