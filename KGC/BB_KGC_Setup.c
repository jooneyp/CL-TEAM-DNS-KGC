#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <gmp.h>
#include <pbc/pbc.h>
#include "BB04.h"
#include "sha2.h"

#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define MAX_INPUT 1024
#define MAX_ID_len 100


// 페어링 생성을 위한 파라미터 선언
static const char *aparam =
	"type a\n"
	"q 8780710799663312522437781984754049815806883199414208211028653399266475630880222957078625179422662221423155858769582317459277713367317481324925129998224791\n"
	"h 12016012264891146079388821366740534204802954401251311822919615131047207289359704531102844802183906537786776\n"
	"r 730750818665451621361119245571504901405976559617\n"
	"exp2 159\n"
	"exp1 107\n"
	"sign1 1\n"
	"sign0 1\n";

void BB_Keygen(BB_SYS_PARAM *bb_param)
{
	int Input_ID_len;
	unsigned char Input_ID[MAX_ID_len];
	FILE *fp;

	if((fp = fopen("Input_Domain_name.txt", "rt")) != NULL) {
		fscanf(fp, "%s", Input_ID);
		Input_ID_len = strlen(Input_ID);
		printf("Input domain name: %s \n", Input_ID);
		printf("Input domain name len: %d\n", Input_ID_len);
	} else {
		printf("%s\n","NO Input_Domain_name.txt");
		system("-1");
	}

	BB_param_import(&bb_param); // 구조체에 파라미터 및 키 등록
	BB_KeyGen_level_1(Input_ID, &bb_param); // 입력 받은 아이디에 대해 1 level의 키 생성 
}

void BB_Hash_1(unsigned char *str, element_t H_1)
{
	unsigned char md[SHA256_DIGEST_SIZE];
	size_t len;

	len = strlen((const char *)str);
	sha256(str, len, md);
	element_from_hash(H_1, md, SHA256_DIGEST_SIZE);
}

// 디랙토리내 My_param폴더에 있는 파라미터값을 열어서 bb_param 구조체에 하나하나 넣어주는 함수 
int BB_param_import(BB_SYS_PARAM *bb_param) 
{
	FILE *fp0, *fp1, *fp2, *fp3, *fp4, *fp5, *fp6, *fp7, *fp8;
	unsigned char buf[MAX_INPUT];

	printf("BB_param_import Start\n");

	fp1 = fopen("My_param/g.param", "rb");
	fp2 = fopen("My_param/g_1.param", "rb");
	fp3 = fopen("My_param/g_2.param", "rb");
	fp4 = fopen("My_param/h_1.param", "rb");
	fp5 = fopen("My_param/h_2.param", "rb");
	fp6 = fopen("My_param/h_3.param", "rb");
	fp7 = fopen("My_param/h_4.param", "rb");
	fp8 = fopen("My_param/h_5.param", "rb");
	//페어링 생성
	pairing_init_set_buf(bb_param->pairing, aparam, strlen(aparam));

	//마스터키가 있는 경우 등록(KGC이므로 있어야함) 
	if((fp0 = fopen("My_param/msk_key.param", "rb")) != NULL) {
		element_init_G1(bb_param->msk_key, bb_param->pairing);
		fread(buf, sizeof(char), 129, fp0);
		element_from_bytes(bb_param->msk_key,buf); buf[0] = '\0';
	    element_printf("bb_param->msk_key\n");
	}

	//변수로 사용하기전에 먼저 init해야함
	element_init_G1(bb_param->g, bb_param->pairing);
	element_init_G1(bb_param->g_1, bb_param->pairing);
	element_init_G1(bb_param->g_2, bb_param->pairing);
	element_init_G1(bb_param->h_1, bb_param->pairing);
	element_init_G1(bb_param->h_2, bb_param->pairing);
	element_init_G1(bb_param->h_3, bb_param->pairing);
	element_init_G1(bb_param->h_4, bb_param->pairing);
	element_init_G1(bb_param->h_5, bb_param->pairing);

	//하나하나 파일로 부터 읽어오고, 원소로 변형
	fread(buf, sizeof(char), 129, fp1);
	element_from_bytes(bb_param->g,buf); buf[0]='\0';

	fread(buf, sizeof(char), 129, fp2);
	element_from_bytes(bb_param->g_1,buf);  buf[0]='\0';

	fread(buf, sizeof(char), 129, fp3);
	element_from_bytes(bb_param->g_2,buf);  buf[0]='\0';

	fread(buf, sizeof(char), 129, fp4);
	element_from_bytes(bb_param->h_1,buf);  buf[0]='\0';

	fread(buf, sizeof(char), 129, fp5);
	element_from_bytes(bb_param->h_2,buf);  buf[0]='\0';

	fread(buf, sizeof(char), 129, fp6);
	element_from_bytes(bb_param->h_3,buf);  buf[0]='\0';

	fread(buf, sizeof(char), 129, fp7);
	element_from_bytes(bb_param->h_4,buf);  buf[0]='\0';

	fread(buf, sizeof(char), 129, fp8);
	element_from_bytes(bb_param->h_5,buf);  buf[0]='\0';

	fclose(fp1);
	fclose(fp2);
	fclose(fp3);
	fclose(fp4);
	fclose(fp5);
	fclose(fp6);
	fclose(fp7);
	fclose(fp8);

	//잘 읽어들였는지 페어링 테스트를 통해 확인
	paring_test(bb_param);
	printf("BB_param_import end\n");

	return 0;
}

// 입력된 아이디에 대하여 1 level의 키를 생성해서 디랙토리내 new_key_level_1 폴더에 저장
void BB_KeyGen_level_1(unsigned char *ID, BB_SYS_PARAM *bb_param) 
{
	FILE *fp1, *fp2;
	unsigned char buf[MAX_INPUT];

	element_t r;
	element_t temp;
	element_t h_ID;

	printf("\nBB_KeyGen_level_1 Start\n");

	fp1 = fopen("new_key_level_1/sk_1.key", "wb");
	fp2 = fopen("new_key_level_1/sk_2.key", "wb");

	element_init_G1(bb_param->sk_1, bb_param->pairing);
	element_init_G1(bb_param->sk_2, bb_param->pairing);
	element_init_Zr(r, bb_param->pairing);
	element_init_G1(temp, bb_param->pairing);
	element_init_G1(h_ID, bb_param->pairing);
	BB_Hash_1(ID, h_ID);

	do{	
		element_random(r);
		element_pow_zn(temp, h_ID, r);
		element_pow_zn(bb_param->sk_2, bb_param->g, r);
		element_mul(bb_param->sk_1, bb_param->msk_key, temp);
	} while 
	((element_len(bb_param->sk_1) < 128) || (element_len(bb_param->sk_2) < 128));


	element_printf("bb_param->sk_1\n");
	element_printf("bb_param->sk_2\n");

	element_to_bytes(buf, bb_param->sk_1);
	fwrite(buf, sizeof(char), (int)strlen(buf), fp1);
	buf[0] = '\0';
	element_to_bytes(buf, bb_param->sk_2);
	fwrite(buf, sizeof(char), (int)strlen(buf), fp2);
	buf[0] = '\0';

	element_clear(r);
	element_clear(temp);
	element_clear(h_ID);
	fclose(fp1);
	fclose(fp2);

	printf("\nBB_KeyGen_level_1 End\n");
}

//기법에서 필요한 파라미터를 생성하는 알고리즘, 생성한 파라미터는 디랙토리내 new_param 폴더 내에 저장됨
void BB_Setup(BB_SYS_PARAM *bb_param)
{
	FILE *fp1, *fp2, *fp3, *fp4, *fp5, *fp6, *fp7, *fp8, *fp9;
	unsigned char buf[MAX_INPUT];
	printf("\nBB_Setup Start (param and msk_key are being generated\n");

	fp1 = fopen("My_param/g.param", "wb");
	fp2 = fopen("My_param/g_1.param", "wb");
	fp3 = fopen("My_param/g_2.param", "wb");
	fp4 = fopen("My_param/h_1.param", "wb");
	fp5 = fopen("My_param/h_2.param", "wb");
	fp6 = fopen("My_param/h_3.param", "wb");
	fp7 = fopen("My_param/h_4.param", "wb");
	fp8 = fopen("My_param/h_5.param", "wb");
	fp9 = fopen("My_param/msk_key.param", "wb"); 

	pairing_init_set_buf(bb_param->pairing, aparam, strlen(aparam));
	element_init_G1(bb_param->g, bb_param->pairing);
	element_init_G1(bb_param->g_1, bb_param->pairing);
	element_init_G1(bb_param->g_2, bb_param->pairing);
	element_init_G1(bb_param->h_1, bb_param->pairing);
	element_init_G1(bb_param->h_2, bb_param->pairing);
	element_init_G1(bb_param->h_3, bb_param->pairing);
	element_init_G1(bb_param->h_4, bb_param->pairing);
	element_init_G1(bb_param->h_5, bb_param->pairing);

	element_init_Zr(bb_param->msk_alpha, bb_param->pairing);
	element_init_G1(bb_param->msk_key, bb_param->pairing);
	
	do { element_random(bb_param->g); } while (element_len(bb_param->g) < 128);
	do { element_random(bb_param->g_2); } while (element_len(bb_param->g_2) < 128);
	do { element_random(bb_param->h_1); } while (element_len(bb_param->h_1) < 128);
	do { element_random(bb_param->h_2); } while (element_len(bb_param->h_2) < 128);
	do { element_random(bb_param->h_3); } while (element_len(bb_param->h_3) < 128);
	do { element_random(bb_param->h_4); } while (element_len(bb_param->h_4) < 128);
	do { element_random(bb_param->h_5); } while (element_len(bb_param->h_5) < 128);
	do { element_random(bb_param->msk_alpha);
 	element_pow_zn(bb_param->g_1, bb_param->g, bb_param->msk_alpha);
	element_pow_zn(bb_param->msk_key, bb_param->g_2, bb_param->msk_alpha);}
	while((element_len(bb_param->g_1) < 128) || (element_len(bb_param->msk_key) < 128));

	paring_test(bb_param);

	element_to_bytes(buf, bb_param->g);
	fwrite(buf, sizeof(char), (int)strlen(buf), fp1); buf[0] = '\0';

	element_to_bytes(buf, bb_param->g_1);
	fwrite(buf, sizeof(char), (int)strlen(buf), fp2); buf[0] = '\0';

	element_to_bytes(buf, bb_param->g_2);
	fwrite(buf, sizeof(char), (int)strlen(buf), fp3); buf[0] = '\0';

	element_to_bytes(buf, bb_param->h_1);
	fwrite(buf, sizeof(char), (int)strlen(buf), fp4); buf[0] = '\0';

	element_to_bytes(buf, bb_param->h_2);
	fwrite(buf, sizeof(char), (int)strlen(buf), fp5); buf[0] = '\0';

	element_to_bytes(buf, bb_param->h_3);
	fwrite(buf, sizeof(char), (int)strlen(buf), fp6); buf[0] = '\0';

	element_to_bytes(buf, bb_param->h_4);
	fwrite(buf, sizeof(char), (int)strlen(buf), fp7); buf[0] = '\0';

	element_to_bytes(buf, bb_param->h_5);
	fwrite(buf, sizeof(char), (int)strlen(buf), fp8); buf[0] = '\0';
	
	element_to_bytes(buf, bb_param->msk_key);
	fwrite(buf, sizeof(char), (int)strlen(buf), fp9); buf[0] = '\0';

	fclose(fp1);
	fclose(fp2);
	fclose(fp3);
	fclose(fp4);
	fclose(fp5);
	fclose(fp6);
	fclose(fp7);
	fclose(fp8);
	fclose(fp9);
	element_clear(bb_param->g);
	element_clear(bb_param->g_1);
	element_clear(bb_param->g_2);
	element_clear(bb_param->h_1);
	element_clear(bb_param->h_2);
	element_clear(bb_param->h_3);
	element_clear(bb_param->h_4);
	element_clear(bb_param->h_5);
	element_clear(bb_param->msk_key);

	printf("BB_Setup End(Parameters stored in My_param)\n\n");
}

// 원소를 바이트로 바꾸었을때 바이트의 길이를 리턴
int element_len(element_t t)
{
	int n;
	unsigned char buf[MAX_INPUT];
	element_to_bytes(buf,t);
    n = (int)strlen(buf);
    buf[0] = '\0';
	return n;
}

// 파라미터로 직접 페어링 연산을 비교해 봄으로써 검증, 성공할 경우 0을 리턴 
int paring_test(BB_SYS_PARAM *bb_param)
{
	element_t temp1;
	element_t temp2;
	element_init_GT(temp1, bb_param->pairing);
	element_init_GT(temp2, bb_param->pairing);
	pairing_apply(temp1, bb_param->g, bb_param->msk_key, bb_param->pairing);
	pairing_apply(temp2, bb_param->g_1, bb_param->g_2, bb_param->pairing);

	if((element_cmp(temp1, temp2)) == 0) {
		element_clear(temp1);
		element_clear(temp2);
		printf("Pairing test succeed\n");
		return 0;
	} else {
		element_clear(temp1);
		element_clear(temp2);
		printf("Pairing test fail\n");
		return 1;
	}
}

int main(int argc, char **argv)
{
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
				BB_Keygen(&bb_param);
				printf("BB_Keygen---------------------------------------\n");
				break;
			case 'e' :
				printf("export\n");
				break;
			case 'i' :
				printf("import\n");
				break;
			case '?' :
				printf("Usage : %s <mode> [option]\n", argv[0]);
				return 0;
		}
	}

	return 0;
}

