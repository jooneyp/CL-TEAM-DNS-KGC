#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <gmp.h>
#include <pbc/pbc.h>
#include "BB04.h"
#include "sha2.h"





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

//스트링을 받아서 원소중 하나로 해쉬하는 함수 (map-to-point 함수)


