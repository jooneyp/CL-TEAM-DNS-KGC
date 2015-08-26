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

// 입력으로 온 ID(domain name)와 자신의ID(domain nane)이 hierarchy한 구조가 맞는지 확인.
int Hierarchy_check(unsigned char *My_ID, unsigned char *Input_ID, int My_ID_len, int Input_ID_len) {
	int i;
	int value=0;

	for(i=0 ; i<My_ID_len ; i++) {
		if(My_ID[My_ID_len-(1+i)] == Input_ID[Input_ID_len-(1+i)])
			value++;
	}
	
	if((My_ID_len == value) && (Input_ID[Input_ID_len-(1+value)] == '.'))
		return 1;
	else
		return 0;
}

//스트링을 받아서 원소중 하나로 해쉬하는 함수 (map-to-point 함수)
void BB_Hash_1(unsigned char *str, element_t H_1) {
	unsigned char md[SHA256_DIGEST_SIZE];
	size_t len;

	len = strlen((const char *)str);

	sha256(str, len, md);

	element_from_hash(H_1, md, SHA256_DIGEST_SIZE);
}

//원소를 바이트로 바꾸었을때 바이트의 길이를 리턴
int element_len(element_t t) {
	int n;
	unsigned char buf[MAX_INPUT];
	element_to_bytes(buf,t);
    n=(int)strlen(buf);
    buf[0]='\0';
	return n;
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


//디랙토리내 My_param폴더에 있는 파라미터값을 열어서 bb_param 구조체에 하나하나 넣어주는 함수 
int BB_param_import(BB_SYS_PARAM *bb_param) {
	FILE *fp0;
	unsigned char buf[MAX_INPUT];

	printf("BB_param_import Start\n");

	pairing_init_set_buf(bb_param->pairing, aparam, strlen(aparam));

	if((fp0 = fopen("My_param/msk_key.param", "rb")) != NULL) {
		element_init_G1(bb_param->msk_key, bb_param->pairing);
		fread(buf, sizeof(char), 129, fp0);
		element_from_bytes(bb_param->msk_key,buf); buf[0] = '\0';
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

    if( paring_test(bb_param) )
    	printf("BB_param_import Succeed\n");
	else
		printf("BB_param_import Fail\n");
	return 0;
}

//디랙토리내 My_sk 폴더에 있는 개인키를 받아와 bb_param에 넣어주는 함수, 결과로 자신의 래밸을 리턴함
int BB_My_Key_Set(BB_SYS_PARAM *bb_param) {
	FILE *fp1, *fp2, *fp3, *fp4, *fp5, *fp6;
	unsigned char buf[MAX_INPUT];

	printf("\n%s\n", "BB_My_Key_Set Start");

	//폴더내에 저장되어 있는 키의 갯수만큼 받아옴, 키의갯수에 따라 래밸이 정해짐 
	if((fp1=fopen("My_sk/sk_1.key","rb")) == NULL) {
		printf("fail to open key file! \n");
		printf("%s\n", "BB_My_Key_Set End!////////////////");

		return 0;
	}
	element_init_G1(bb_param->sk_1, bb_param->pairing);
	fread(buf, sizeof(char), 129, fp1);
	element_from_bytes(bb_param->sk_1,buf); buf[0]='\0';
   // element_printf("\nbb_param->sk_1 : %B\n\n", bb_param->sk_1);
	fclose(fp1);
	

	if((fp2=fopen("My_sk/sk_2.key","rb")) == NULL) {
		printf("fail to open key file!\n");
		printf("%s\n", "BB_My_Key_Set End!////////////////");

		return 0;
	}
	element_init_G1(bb_param->sk_2, bb_param->pairing);
	fread(buf, sizeof(char), 129, fp2);
	element_from_bytes(bb_param->sk_2,buf); buf[0]='\0';
   // element_printf("\nbb_param->sk_2 : %B\n\n", bb_param->sk_2);
	fclose(fp2);
	

	if((fp3=fopen("My_sk/sk_3.key","rb")) == NULL)
	{
		printf("Your Key is 1 level(2 keys)\n");
		printf("%s\n", "BB_My_Key_Set End!////////////////");

		return 1;
	}	
	element_init_G1(bb_param->sk_3, bb_param->pairing);
	fread(buf,sizeof(char),129,fp3);
	element_from_bytes(bb_param->sk_3,buf); buf[0]='\0';
	fclose(fp3);
	

	if((fp4=fopen("My_sk/sk_4.key","rb"))==NULL)
	{
		printf("Your Key is 2 level(3 keys) \n");
		printf("%s\n", "BB_My_Key_Set End!////////////////");

		return 2;
	}	
	element_init_G1(bb_param->sk_4, bb_param->pairing);
	fread(buf,sizeof(char),129,fp4);
	element_from_bytes(bb_param->sk_4,buf); buf[0]='\0';
	fclose(fp4);
	

	if((fp5=fopen("My_sk/sk_5.key","rb"))==NULL) {
		printf("Your Key is 3 level(4 keys)\n");
		printf("%s\n", "BB_My_Key_Set End!////////////////");

		return 3;
	}	
	element_init_G1(bb_param->sk_5, bb_param->pairing);
	fread(buf,sizeof(char),129,fp5);
	element_from_bytes(bb_param->sk_5,buf); buf[0]='\0';
	fclose(fp5);
	

	if((fp6=fopen("My_sk/sk_6.key","rb"))==NULL) {
		printf("Your Key is 4 level(5 keys) \n");
		printf("%s\n", "BB_My_Key_Set End!////////////////");

		return 4;
	}	
	element_init_G1(bb_param->sk_6, bb_param->pairing);
	fread(buf,sizeof(char),129,fp6);
	element_from_bytes(bb_param->sk_6,buf); buf[0]='\0';
	fclose(fp6);

	printf("Your Key is 5 level(6 keys) \n");
	printf("%s\n\n", "BB_My_Key_Set End!////////////////");
		
	return 5;
}


//파라미터로 직접 페어링 연산을 비교해 봄으로써 검증, 성공할 경우 0을 리턴 
int paring_test(BB_SYS_PARAM *bb_param)
{
	element_t r;
	element_t temp;
	element_t temp1;
	element_t temp2;

	element_init_Zr(r, bb_param->pairing);
	element_init_G1(temp, bb_param->pairing);
	element_init_GT(temp1, bb_param->pairing);
	element_init_GT(temp2, bb_param->pairing);

	element_random(r);

	element_pow_zn(temp,bb_param->g_1,r);
	pairing_apply(temp1,bb_param->g,temp,bb_param->pairing);
	element_pow_zn(temp,bb_param->g,r);
	pairing_apply(temp2,bb_param->g_1,temp,bb_param->pairing);

	if((element_cmp(temp1,temp2))==0)
	{
	element_clear(r);
	element_clear(temp);
	element_clear(temp1);
	element_clear(temp2);
	printf("%s\n", "Pairing test succeed!!");
	return 0;
	}

	else
	element_clear(r);
	element_clear(temp);
	element_clear(temp1);
	element_clear(temp2);
	printf("%s\n", "Pairing test fail!!(something wrong)");
	return 1;
}


// 입력받은 아이디에 키를 생성해 주는 알고리즘 
int BB_KeyGen_level_n(unsigned char *ID, BB_SYS_PARAM *bb_param, int my_key_level)
{

	FILE *fp0, *fp1, *fp2, *fp3, *fp4, *fp5, *fp6;
	unsigned char *fname;
	unsigned char buf[MAX_INPUT];
	element_t r;
	element_t temp;
	element_t h_ID;


	printf("\n%s\n", "BB_KeyGen_level_n Start!////////////////");

	if (my_key_level == 1) 
		fname = "new_key/sk_3.key";
	if (my_key_level == 2) 
		fname = "new_key/sk_4.key";
	if (my_key_level == 3) 
		fname = "new_key/sk_5.key";
	if (my_key_level == 4) 
		fname = "new_key/sk_6.key";

	
	fp0 = fopen(fname, "wb");

	element_init_Zr(r, bb_param->pairing);
	element_init_G1(temp, bb_param->pairing);
	element_init_G1(h_ID, bb_param->pairing);
	element_init_G1(bb_param->sk_0, bb_param->pairing);

	BB_Hash_1(ID, h_ID);


	if (my_key_level == 1) { //자기 래밸이 1인 경우 
		element_init_G1(bb_param->sk_3, bb_param->pairing);
		do {
			element_random(r);
			element_pow_zn(temp, h_ID,r);
			element_mul(bb_param->sk_0, bb_param->sk_1, temp);
			element_pow_zn(bb_param->sk_3, bb_param->g, r);
	    } while ( (element_len(bb_param->sk_0) < 128) || (element_len(bb_param->sk_3) < 128) );
	    
	    fp1 = fopen("new_key/sk_1.key", "wb");
		fp2 = fopen("new_key/sk_2.key", "wb");
		fp3 = fopen("new_key/sk_3.key", "wb");

	    element_to_bytes(buf, bb_param->sk_0);
		fwrite(buf, sizeof(char), (int)strlen(buf), fp1); buf[0]='\0';
		element_to_bytes(buf, bb_param->sk_2);
		fwrite(buf, sizeof(char), (int)strlen(buf), fp2); buf[0]='\0';
		element_to_bytes(buf, bb_param->sk_3);
		fwrite(buf, sizeof(char), (int)strlen(buf), fp3); buf[0]='\0';
		printf("%s\n", "new key is 2 level(3 keys)");
	}
	
	if (my_key_level == 2) { //자기 래밸이 2인 경우 
		element_init_G1(bb_param->sk_4, bb_param->pairing);		
		do {
			element_random(r);
			element_pow_zn(temp, h_ID, r);
			element_mul(bb_param->sk_0, bb_param->sk_1, temp);
			element_pow_zn(bb_param->sk_4, bb_param->g, r);
	    } while ((element_len(bb_param->sk_0) < 128) || (element_len(bb_param->sk_4) < 128));

	    fp1 = fopen("new_key/sk_1.key","wb");
		fp2 = fopen("new_key/sk_2.key","wb");
		fp3 = fopen("new_key/sk_3.key","wb");
		fp4 = fopen("new_key/sk_4.key","wb");

	    element_to_bytes(buf, bb_param->sk_0);
		fwrite(buf, sizeof(char), (int)strlen(buf), fp1); buf[0]='\0';
		element_to_bytes(buf, bb_param->sk_2);
		fwrite(buf, sizeof(char), (int)strlen(buf), fp2); buf[0]='\0';
		element_to_bytes(buf, bb_param->sk_3);
		fwrite(buf, sizeof(char), (int)strlen(buf), fp3); buf[0]='\0';
		element_to_bytes(buf, bb_param->sk_4);
		fwrite(buf, sizeof(char), (int)strlen(buf), fp4); buf[0]='\0';
	    printf("%s\n", "new key is 3 level(4 keys)");
	}
	
	if (my_key_level == 3) { //자기 래밸이 3인 경우 
		element_init_G1(bb_param->sk_5, bb_param->pairing);		
		do{
			element_random(r);
			element_pow_zn(temp,h_ID,r);
			element_mul(bb_param->sk_0, bb_param->sk_1, temp);
			element_pow_zn(bb_param->sk_5, bb_param->g, r);
	    } while ((element_len(bb_param->sk_0)<128) || (element_len(bb_param->sk_5)<128));

	    fp1=fopen("new_key/sk_1.key","wb");
		fp2=fopen("new_key/sk_2.key","wb");
		fp3=fopen("new_key/sk_3.key","wb");
		fp4=fopen("new_key/sk_4.key","wb");
		fp5=fopen("new_key/sk_5.key","wb");

	    element_to_bytes(buf,bb_param->sk_0);
		fwrite(buf,sizeof(char),(int)strlen(buf),fp1);buf[0]='\0';
		//element_printf("\nbb_param->sk_1(new key) : %B\n", bb_param->sk_0);
		element_to_bytes(buf,bb_param->sk_2);
		fwrite(buf,sizeof(char),(int)strlen(buf),fp2);buf[0]='\0';
		//element_printf("\nbb_param->sk_2(new key) : %B\n", bb_param->sk_2);
		element_to_bytes(buf,bb_param->sk_3);
		fwrite(buf,sizeof(char),(int)strlen(buf),fp3);buf[0]='\0';
	    //element_printf("\nbb_param->sk_3(new key) : %B\n", bb_param->sk_3);
		element_to_bytes(buf,bb_param->sk_4);
		fwrite(buf,sizeof(char),(int)strlen(buf),fp4);buf[0]='\0';
	   // element_printf("\nbb_param->sk_4(new key) : %B\n", bb_param->sk_4);
		element_to_bytes(buf,bb_param->sk_5);
		fwrite(buf,sizeof(char),(int)strlen(buf),fp5);buf[0]='\0';
	    //element_printf("\nbb_param->sk_5(new key) : %B\n", bb_param->sk_5);
	    printf("%s\n", "new key is 4 level(5 keys)");
	}
	
	if (my_key_level==4)//자기 래밸이 4인 경우 
	{
			element_init_G1(bb_param->sk_6, bb_param->pairing);	
		do {
		element_random(r);
		element_pow_zn(temp,h_ID,r);
		element_mul(bb_param->sk_0, bb_param->sk_1, temp);
		element_pow_zn(bb_param->sk_6, bb_param->g, r);
	    } while ((element_len(bb_param->sk_0)<128) || (element_len(bb_param->sk_6)<128));

	    fp1=fopen("new_key/sk_1.key","wb");
		fp2=fopen("new_key/sk_2.key","wb");
		fp3=fopen("new_key/sk_3.key","wb");
		fp4=fopen("new_key/sk_4.key","wb");
		fp5=fopen("new_key/sk_5.key","wb");
		fp6=fopen("new_key/sk_6.key","wb");

	    element_to_bytes(buf,bb_param->sk_0);
		fwrite(buf,sizeof(char),(int)strlen(buf),fp1);buf[0]='\0';
		element_to_bytes(buf,bb_param->sk_2);
		fwrite(buf,sizeof(char),(int)strlen(buf),fp2);buf[0]='\0';
		element_to_bytes(buf,bb_param->sk_3);
		fwrite(buf,sizeof(char),(int)strlen(buf),fp3);buf[0]='\0';
		element_to_bytes(buf,bb_param->sk_4);
		fwrite(buf,sizeof(char),(int)strlen(buf),fp4);buf[0]='\0';
		element_to_bytes(buf,bb_param->sk_5);
		fwrite(buf,sizeof(char),(int)strlen(buf),fp5);buf[0]='\0';
		element_to_bytes(buf,bb_param->sk_6);
		fwrite(buf,sizeof(char),(int)strlen(buf),fp6);buf[0]='\0';
	    printf("%s\n", "new key is 5 level(6 keys)");
	}

	if (my_key_level == 5)
		printf("%s\n","you can't generate key (you are in lowest level:5)" );//자기 래밸이 5인 경우, 래밸 5가 최대이므로 더 이상 생성하지 못함  

	element_clear(r);
	element_clear(temp);
	element_clear(h_ID);
	fclose(fp1);
	fclose(fp2);
	printf("%s\n\n", "BB_KeyGen_level_n End!////////////////");

	return 0;

}


int main(int argc, char **argv)
{

	int My_ID_len;
	int Input_ID_len;
	int my_key_level;
	int n;
	unsigned char My_ID[MAX_ID_len];
	unsigned char Input_ID[MAX_ID_len];
	unsigned char ID[MAX_ID_len];

	BB_SYS_PARAM bb_param;
	FILE *fp1,*fp2;

	//자신의 ID(domain name)를 디랙토리내 My_Domain_name.txt 파일로 입력 받아 My_ID에 저장 
	if((fp1=fopen("My_Domain_name.txt","rt"))==NULL)
	{printf("%s\n","NO My_Domain_name.txt" ); return 1;}
	fscanf(fp1,"%s",My_ID);
	My_ID_len=strlen(My_ID);
	printf("\nyour domain name: %s \n",My_ID);
	printf("your domain name len: %d\n\n",My_ID_len);

	//키를 생성할 ID를 디랙토리내 Input_Domain_name.txt 파일로 입력 받아 Input_ID에 저장 
	if((fp2=fopen("Input_Domain_name.txt","rt"))!=NULL)
	{
	fscanf(fp2,"%s",Input_ID);
	Input_ID_len=strlen(Input_ID);
	printf("Input domain name: %s \n",Input_ID);
	printf("Input domain name len: %d\n\n",Input_ID_len);
	}
	else{printf("%s\n","NO Input_Domain_name.txt");}

	//입력으로 온 ID(domain name)와 자신의ID(domain nane)이 hierarchy한 구조가 맞는지 확인.
	if(Hierarchy_check(My_ID,Input_ID,My_ID_len,Input_ID_len)==0)
	{printf("\n%s\n","UNMATCHED HIERARCHY!!");return 1;}
	else
	printf("%s\n","HIERARCHY CONFORMED!!");
	strncpy(ID,Input_ID,(Input_ID_len-My_ID_len));//입력으로 들어온 ID에서 자신의 아이디 부분 제거 ex Input_ID:google.com, My_ID:com -> ID:google.(BB04 스킴의 특징)
	

  	if((BB_param_import(&bb_param))==1)
  	return 1;//My_param폴더에 저장되어 있는 파라미터값을 불러옴 
  	my_key_level=BB_My_Key_Set(&bb_param);//My_sk폴더에 저장되어 있는 개인키 값을 불러옴, 개인키의 갯수에 따라 자신의 래밸이 결정됨
  	BB_KeyGen_level_n(ID, &bb_param,my_key_level);//ID 차이에 대한 개인키 생성 
   	// must after BB_param_import

	
	
	return 0;
}
