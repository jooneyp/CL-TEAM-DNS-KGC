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
#include <BB_Setup_Keygen.h>

int main(int argv, char **argv) {
	BB_SYS_PARAM bb_param;
	int param_opt;

	while (-1 != (param_opt = getopt(argc, argv, "skei"))) {
		switch(param_opt)
		{
			case 's' :
				printf("BB_Setup---------------------------------------\n");
				BB_Setup(&bb_param);
				printf("BB_Setup---------------------------------------\n");
				break;
			case 'k' :
				printf("BB_Keygen---------------------------------------\n");
				if(argc == 3)
					BB_Keygen(argv[2], &bb_param);
				else
					printf("Usage : %s <Domain Name>\n", argv[1]);
				printf("BB_Keygen---------------------------------------\n");
				break;
			case 'e' :
				printf("export\n");
				if(argc == 3)
					Export(argv[2], argv[3]);
				else
					printf("Usage : %s <Target IP> <Domain Name>\n", argv[1]);
				break;
			case 'i' :
				printf("import\n");
				break;
			case '?' :
				printf("Usage : %s <mode (skei)> [option]\n", argv[0]);
				return 0;
		}
	}

	return 0;
}