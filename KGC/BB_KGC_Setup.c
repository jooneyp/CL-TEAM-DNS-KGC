#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <gmp.h>
#include <pbc/pbc.h>
#include "BB04.h"

#define MAX_INPUT 1024


//페어링 생성을 위한 파라미터 선언
static const char *aparam =
	"type a\n"
	"q 8780710799663312522437781984754049815806883199414208211028653399266475630880222957078625179422662221423155858769582317459277713367317481324925129998224791\n"
	"h 12016012264891146079388821366740534204802954401251311822919615131047207289359704531102844802183906537786776\n"
	"r 730750818665451621361119245571504901405976559617\n"
	"exp2 159\n"
	"exp1 107\n"
	"sign1 1\n"
	"sign0 1\n";


//원소를 바이트로 바꾸었을때 바이트의 길이를 리턴
int element_len(element_t t)
{
	int n;
	unsigned char buf[MAX_INPUT];
	element_to_bytes(buf,t);
    n=(int)strlen(buf);
    buf[0]='\0';
	return n;
}

//기법에서 필요한 파라미터를 생성하는 알고리즘, 생성한 파라미터는 디랙토리내 new_param 폴더 내에 저장됨
void BB_Setup(BB_SYS_PARAM *bb_param)
{


	FILE *fp1,*fp2,*fp3,*fp4,*fp5,*fp6,*fp7,*fp8,*fp9;
	unsigned char buf[MAX_INPUT];
	printf("\nBB_Setup Start (new param and msk_key are being generated!!///////// \n");

	fp1=fopen("new_param/g.param","wb");
	fp2=fopen("new_param/g_1.param","wb");
	fp3=fopen("new_param/g_2.param","wb");
	fp4=fopen("new_param/h_1.param","wb");
	fp5=fopen("new_param/h_2.param","wb");
	fp6=fopen("new_param/h_3.param","wb");
	fp7=fopen("new_param/h_4.param","wb");
	fp8=fopen("new_param/h_5.param","wb");
	fp9=fopen("new_param/msk_key.param","wb"); 

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
	
	do{element_random(bb_param->g);}while(element_len(bb_param->g)<128);
	do{element_random(bb_param->g_2);}while(element_len(bb_param->g_2)<128);
	do{element_random(bb_param->h_1);}while(element_len(bb_param->h_1)<128);
	do{element_random(bb_param->h_2);}while(element_len(bb_param->h_2)<128);
	do{element_random(bb_param->h_3);}while(element_len(bb_param->h_3)<128);
	do{element_random(bb_param->h_4);}while(element_len(bb_param->h_4)<128);
	do{element_random(bb_param->h_5);}while(element_len(bb_param->h_5)<128);
	do{element_random(bb_param->msk_alpha);
	element_pow_zn(bb_param->g_1, bb_param->g, bb_param->msk_alpha);
	element_pow_zn(bb_param->msk_key, bb_param->g_2, bb_param->msk_alpha);}
	while((element_len(bb_param->g_1)<128)||(element_len(bb_param->msk_key)<128));

	paring_test(bb_param);


	element_to_bytes(buf,bb_param->g);
	fwrite(buf,sizeof(char),(int)strlen(buf),fp1);buf[0]='\0';

	element_to_bytes(buf,bb_param->g_1);
	fwrite(buf,sizeof(char),(int)strlen(buf),fp2);buf[0]='\0';

	element_to_bytes(buf,bb_param->g_2);
	fwrite(buf,sizeof(char),(int)strlen(buf),fp3);buf[0]='\0';

	element_to_bytes(buf,bb_param->h_1);
	fwrite(buf,sizeof(char),(int)strlen(buf),fp4);buf[0]='\0';

	element_to_bytes(buf,bb_param->h_2);
	fwrite(buf,sizeof(char),(int)strlen(buf),fp5);buf[0]='\0';

	element_to_bytes(buf,bb_param->h_3);
	fwrite(buf,sizeof(char),(int)strlen(buf),fp6);buf[0]='\0';

	element_to_bytes(buf,bb_param->h_4);
	fwrite(buf,sizeof(char),(int)strlen(buf),fp7);buf[0]='\0';

	element_to_bytes(buf,bb_param->h_5);
	fwrite(buf,sizeof(char),(int)strlen(buf),fp8);buf[0]='\0';
	
	element_to_bytes(buf,bb_param->msk_key);
	fwrite(buf,sizeof(char),(int)strlen(buf),fp9);buf[0]='\0';

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

	printf("\nBB_Setup End(stored in new_param)////////////////////////// \n");




}

//파라미터로 직접 페어링 연산을 비교해 봄으로써 검증, 성공할 경우 0을 리턴 
int paring_test(BB_SYS_PARAM *bb_param)
{
	element_t temp1;
	element_t temp2;
	element_init_GT(temp1, bb_param->pairing);
	element_init_GT(temp2, bb_param->pairing);
	pairing_apply(temp1,bb_param->g,bb_param->msk_key,bb_param->pairing);
	pairing_apply(temp2,bb_param->g_1,bb_param->g_2,bb_param->pairing);

	if((element_cmp(temp1,temp2))==0)
	{
	element_clear(temp1);
	element_clear(temp2);
	printf("\n%s\n", "Pairing test succeed!!");
	return 0;
	}
	else
	element_clear(temp1);
	element_clear(temp2);
	printf("\n%s\n", "Pairing test fail!!(something wrong)");
	return 1;
}



int main()
{

	BB_SYS_PARAM bb_param;
	BB_Setup(&bb_param);

	return 0;
}
