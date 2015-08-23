#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
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
#define MAX_ID_LEN 100

static const char *aparam =
	"type a\n"
	"q 8780710799663312522437781984754049815806883199414208211028653399266475630880222957078625179422662221423155858769582317459277713367317481324925129998224791\n"
	"h 12016012264891146079388821366740534204802954401251311822919615131047207289359704531102844802183906537786776\n"
	"r 730750818665451621361119245571504901405976559617\n"
	"exp2 159\n"
	"exp1 107\n"
	"sign1 1\n"
	"sign0 1\n";

void BB_Keygen(unsigned char *Input_ID, BB_SYS_PARAM *bb_param)
{
	int Input_ID_len;
	char ID[MAX_ID_LEN];
	Input_ID_len = strlen(Input_ID);
	printf("\nInput domain name: %s\n", Input_ID);
	printf("Input domain name len: %d\n", Input_ID_len);

	BB_param_import(bb_param);
	BB_KeyGen_level_1(ID, bb_param);
}

void BB_Hash_1(unsigned char *str, element_t H_1)
{
	unsigned char md[SHA256_DIGEST_SIZE];
	size_t len;

	len = strlen((const char *)str);
	sha256(str, len, md);
	element_from_hash(H_1, md, SHA256_DIGEST_SIZE);
}

void BB_import_File_Controller(element_t e, char *filename) {
	FILE *fp;
	unsigned char buf[MAX_INPUT];

	if(fp = fopen(filename, "rb") == NULL){
		printf("ERROR : check folder created!\n");
	}

	fread(buf, sizeof(char), 129, fp);
	element_from_bytes(e, buf);

	fclose(fp);	
}

// 디랙토리내 My_param폴더에 있는 파라미터값을 열어서 bb_param 구조체에 하나하나 넣어주는 함수 
int BB_param_import(BB_SYS_PARAM *bb_param) 
{
	FILE *fp0;
	unsigned char buf[MAX_INPUT];

	printf("BB_param_import Start\n");
	//페어링 생성
	pairing_init_set_buf(bb_param->pairing, aparam, strlen(aparam));

	//마스터키가 있는 경우 등록(KGC이므로 있어야함) 
	if((fp0 = fopen("My_param/msk_key.param", "rb")) != NULL) {
		element_init_G1(bb_param->msk_key, bb_param->pairing);
		fread(buf, sizeof(char), 129, fp0);
		element_from_bytes(bb_param->msk_key,buf); buf[0] = '\0';
	    element_printf("bb_param->msk_key\n");
	}

	element_init_G1(bb_param->g, bb_param->pairing);
	element_init_G1(bb_param->g_1, bb_param->pairing);
	element_init_G1(bb_param->g_2, bb_param->pairing);
	element_init_G1(bb_param->h_1, bb_param->pairing);
	element_init_G1(bb_param->h_2, bb_param->pairing);
	element_init_G1(bb_param->h_3, bb_param->pairing);
	element_init_G1(bb_param->h_4, bb_param->pairing);
	element_init_G1(bb_param->h_5, bb_param->pairing);

	BB_import_File_Controller(bb_param->g, "My_param/g.param");
	BB_import_File_Controller(bb_param->g_1, "My_param/g_1.param");
	BB_import_File_Controller(bb_param->g_2, "My_param/g_2.param");
	BB_import_File_Controller(bb_param->h_1, "My_param/h_1.param");
	BB_import_File_Controller(bb_param->h_2, "My_param/h_2.param");
	BB_import_File_Controller(bb_param->h_3, "My_param/h_3.param");
	BB_import_File_Controller(bb_param->h_4, "My_param/h_4.param");
	BB_import_File_Controller(bb_param->h_5, "My_param/h_5.param");

	//잘 읽어들였는지 페어링 테스트를 통해 확인
	paring_test(bb_param);
	printf("BB_param_import end\n");

	return 0;
}

// 입력된 아이디에 대하여 1 level의 키를 생성해서 디랙토리내 new_key_level_1 폴더에 저장
int BB_KeyGen_level_1(unsigned char *ID, BB_SYS_PARAM *bb_param) 
{
	FILE *fp1, *fp2;
	unsigned char buf[MAX_INPUT];

	element_t r;
	element_t temp;
	element_t h_ID;

	printf("\nBB_KeyGen_level_1 Start\n");

	fp1 = fopen("new_key_level_1/sk_1.key", "wb");
	fp2 = fopen("new_key_level_1/sk_2.key", "wb");

	if (fp1 == NULL || fp2 == NULL) {
		printf("ERROR : check 'new_key_level_1' folder exist\n");
		exit(-1);
	}

	element_init_G1(bb_param->sk_1, bb_param->pairing);
	element_init_G1(bb_param->sk_2, bb_param->pairing);
	element_init_Zr(r, bb_param->pairing);
	element_init_G1(temp, bb_param->pairing);
	element_init_G1(h_ID, bb_param->pairing);
	BB_Hash_1(ID, h_ID);

	do {	
		element_random(r);
		element_pow_zn(temp, h_ID, r);
		element_pow_zn(bb_param->sk_2, bb_param->g, r);
		element_mul(bb_param->sk_1, bb_param->msk_key, temp);
	} while 
		((element_len(bb_param->sk_1) < 128) || (element_len(bb_param->sk_2) < 128));


	element_printf("bb_param->sk_1 : %B\n", bb_param->sk_1);
	element_printf("bb_param->sk_2 : %B\n", bb_param->sk_2);

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

	return 0;
}

void BB_Setup_File_Controller(element_t e, char *filename) {
	FILE *fp;
	unsigned char buf[MAX_INPUT];

	if(fp = fopen(filename, "wb") == NULL) {
		printf("ERROR : check folder created!\n");
		exit(-1);
	}
	element_to_bytes(buf, e);
	fwrite(buf, sizeof(char), (int)strlen(buf), fp);

	fclose(fp);	
}

// 기법에서 필요한 파라미터를 생성하는 알고리즘, 생성한 파라미터는 디랙토리내 My_param 폴더 내에 저장됨
void BB_Setup(BB_SYS_PARAM *bb_param)
{
	unsigned char buf[MAX_INPUT];
	printf("\nBB_Setup Start (param / msk_key generated)\n");

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

	BB_Setup_File_Controller(bb_param->g, "My_param/g.param");
	BB_Setup_File_Controller(bb_param->g_1, "My_param/g_1.param");
	BB_Setup_File_Controller(bb_param->g_2, "My_param/g_2.param");
	BB_Setup_File_Controller(bb_param->h_1, "My_param/h_1.param");
	BB_Setup_File_Controller(bb_param->h_2, "My_param/h_2.param");
	BB_Setup_File_Controller(bb_param->h_3, "My_param/h_3.param");
	BB_Setup_File_Controller(bb_param->h_4, "My_param/h_4.param");
	BB_Setup_File_Controller(bb_param->h_5, "My_param/h_5.param");
	BB_Setup_File_Controller(bb_param->msk_key, "My_param/msk_key.param");

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
