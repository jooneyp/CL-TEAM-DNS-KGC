#include <pbc/pbc.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HASH_LENGTH_128 32
#define HASH_LENGTH_80 20


typedef struct bb_param_st{
	element_t g;
	element_t g_1;
	element_t g_2;
	element_t h_1;
	element_t h_2;
	element_t h_3;
	element_t h_4;
	element_t h_5;
	element_t msk_key;
	element_t msk_alpha;
	element_t sk_0;
	element_t sk_1;
	element_t sk_2;
	element_t sk_3;
	element_t sk_4;
	element_t sk_5;
	element_t sk_6;
	pairing_t pairing;
}BB_SYS_PARAM;

typedef struct bb_ciphertext_st{
	unsigned char c[HASH_LENGTH_80];
	element_t c_0;
	element_t c_1;
	element_t t;
}BB_CT;


void paramclear_bb(BB_SYS_PARAM *bb_param);


#ifdef __cplusplus
}
#endif
