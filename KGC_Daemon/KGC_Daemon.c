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

void error_handling(char * msg);
void receive_ip_url();
void send_params();
void named_conf();

int clnt_cnt=0;
int clnt_socks[MAX_CLNT];

char clnt_IP[20];
char URL[BUF_SIZE];
char *IP;

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
				while(1) {
					printf("BB KGC Daemon Start\n");
					receive_ip_url();
					printf("BB_Keygen Start\n");
					BB_Keygen(URL, &bb_param);
					printf("BB_Keygen END\n");
					sleep(1);
					printf("Sending Parameters...\n");
					send_params();
					printf("Parameters Sent, Configuring...\n");
					named_conf();
					printf("Configure END.\n\n");
				}
				break;
			case '?' :
				printf("Usage : %s <mode (sd)> [option]\n", argv[0]);
				return 0;
		}
	}

	return 0;
}

void receive_ip_url() {
	int serv_sock, clnt_sock;
	struct sockaddr_in serv_adr, clnt_adr;
	int clnt_adr_sz;
	int option = 1;
	int size;

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family=AF_INET; 
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(5959);
	
	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1)
		error_handling("bind() error");
	if(listen(serv_sock, 5) == -1)
		error_handling("listen() error");

	setsockopt( serv_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option) );

	clnt_adr_sz = sizeof(clnt_adr);
	clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

	strncpy(clnt_IP, inet_ntoa(clnt_adr.sin_addr), strlen(inet_ntoa(clnt_adr.sin_addr)));
	printf("\nKey Generation Request from IP : %s\n", clnt_IP);

	read(clnt_sock, URL, sizeof(URL)); // 들어오는 URL과 IP를 URL에 받는다.
	IP = strtok(URL, "^");
	printf("Received URL, IP : %s, %s\n\n", URL, IP);
	close(serv_sock);
	close(clnt_sock);
	// printf("IP/URL Receiver socket closed\n");
}

void send_params() {
	int retval;
	int sock;
	int i;
	int count = 14;
	struct sockaddr_in serveraddr;

	char filename[14][256] = {
		"g.param", "g_1.param", "g_2.param", "h_1.param", "h_2.param", "h_3.param", "h_4.param", "h_5.param",
		"sk_1.key", "sk_2.key", "sk_3.key", "sk_4.key", "sk_5.key", "sk_6.key"
	};
	FILE *fp;
	char fullname[256];

	// 순차적으로 있는 파마미터 및 키를 모두 전송 
	for (i = 0; i < count; i++)
	{
		if(filename[i][0] != 's') {
			strncpy(fullname, "My_param/", 10);
		} else {
			strncpy(fullname, "new_key_level_1/", 17);
		}
		strncat(fullname, filename[i], strlen(fullname) + strlen(filename[i]));
		if((fp = fopen(fullname, "rb")) != NULL) {

			sock = socket(PF_INET, SOCK_STREAM, 0);
			if (sock == -1)
				error_handling("socket() error");
			memset(&serveraddr, 0, sizeof(serveraddr));
			serveraddr.sin_family = AF_INET;
			serveraddr.sin_port = htons(5960);
			serveraddr.sin_addr.s_addr = inet_addr(clnt_IP);
			retval = connect(sock, (struct sockaddr*) &serveraddr, sizeof(serveraddr));

			if(retval == -1)
				error_handling("connect() error");
		
			printf("filename: %s\n", filename[i]);
			retval = write(sock, fullname, sizeof(fullname));
		
			if(retval == -1)
				error_handling("write() error1");
	 
			fseek(fp, 0, SEEK_END);
			int totalbytes = ftell(fp);
	 
			retval = write(sock, (char *)&totalbytes, sizeof(totalbytes));
			if(retval == -1)
				error_handling("write() error2");
	 
			char buf[BUF_SIZE];
			int numread;
			int numtotal = 0;
	 
			rewind(fp);
			while(1) {
				numread = fread(buf, 1, BUF_SIZE, fp);
				if(numread > 0) {
					retval = write(sock, buf, numread);
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
			close(sock);
			buf[0]='\0';
		} else {
			sock = socket(PF_INET, SOCK_STREAM, 0);
			if (sock == -1)
				error_handling("socket() error");
			memset(&serveraddr, 0, sizeof(serveraddr));
			serveraddr.sin_family = AF_INET;
			serveraddr.sin_port = htons(5960);
			serveraddr.sin_addr.s_addr = inet_addr(clnt_IP);
			retval = connect(sock, (struct sockaddr*) &serveraddr, sizeof(serveraddr));
			write(sock, "END", strlen("END"));
			printf("%s not found. closing socket.\n", filename[i]);
			i = count;
			close(sock);
		}
	}
}

void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}

void named_conf() {
	FILE *fp;
	char text[BUF_SIZE];
	if((fp = fopen("/etc/named.rfc1912.zones", "wt")) == NULL) {
		printf("fopen() for named.rfc1912.zones ERROR\n");
	} else {
		text[0] = '\0';

		strcat(text, "\nzone \""); strcat(text, URL); strcat(text, "\" IN {\n");
		strcat(text, "\ttype master;\n");
		strcat(text, "\tfile \""); strcat(text, URL); strcat(text, ".zone\";\n");
		strcat(text, "\tallow-update { none; };\n");
		strcat(text, "}\n");

		fprintf(fp, "\n\n%s\n", text);

		printf("Added new line -----\n\n%s\n", text);
	}
	fclose(fp);
}