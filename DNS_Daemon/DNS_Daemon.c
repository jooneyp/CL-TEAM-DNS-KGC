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
void error_handling(char * msg);
void open_socket();

int clnt_cnt=0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;
char clnt_IP[20];
char URL[BUF_SIZE];

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

