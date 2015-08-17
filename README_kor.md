Test

사전 준비
gmp, pbc 라이브러리 설치

컴파일
//KGC
gcc -o BB_KGC_Setup sha2.c BB_KGC_Setup.c -lgmp -lpbc
gcc -o BB_KGC_Keygen sha2.c BB_KGC_Keygen.c -lgmp -lpbc

//DNS
gcc -o BB_DNS_Keygen sha2.c BB_DNS_Keygen.c -lgmp -lpbc

//공통(import,export)
gcc -o import import.c
gcc -o export_ID export_ID.c
gcc -o export_param_and_key export_param_and_key.c


시연?순서(폴더생성과 파라미터값 이동을 제외하고는 꼭 순서 따라야하는것은 아님)

-KGC 키생성 

1. BB_KGC_Setup 실행하면 new_param 에 8개의 파라미터와 1개의 마스터키 생성됨
2. 9개의 파라미터와 마스터키 모두를 My_param폴더에 이동


-DNS가 KGC로 부터 키발급

1. DNS에서 Input_Domain_name.txt에 발급받고자 하는 ID입력하고 EXPORT폴더에 이동후 EXPORT_ID 실행(KGC는 IMPORT실행중)

2. KGC에서 받은 Input_Domain_name.txt파일(IMPORT 폴더에 있음)을 KGC 폴더로 이동후 BB_KGC_Keygen실행, 새로운 키들이 new_key_level_1에 저장됨

3. KGC에서 My_param폴더내 8개의 파라미터(9개중 마스터키 제외)와 new_key_level_1 폴더에 생성된 키들을 EXPORT 폴더에 이동후, EXPORT_param_and_key 실행(DNS는 IMPORT실행중)

4. DNS는 IMPORT폴더에서 받은 값들중 8개의 파라미터는 My_param에 넣고 Key는 My_sk에 넣어준다.


-DNS*가 DNS로 부터 키발급 받기

1. DNS*에서 Input_Domain_name.txt에 발급받고자 하는 ID입력하고 EXPORT폴더에 이동후 EXPORT_ID 실행(DNS는 IMPORT실행중)

2. DNS에서 받은 Input_Domain_name.txt파일(IMPORT 폴더에 있음)을 DNS폴더로 이동, BB_DNS_Keygen실행(My_Domain_name.txt에는 자신의 도메인이 들어가 있어여함), 새로운 키들이 new_key에 저장됨

3. DNS에서 My_param폴더내 8개의 파라미터와 new_key폴더에 생성된 키들을 EXPORT 폴더에 이동후, EXPORT_param_and_key 실행(DNS*는 IMPORT실행중)

4. DNS*는 IMPORT폴더에서 받은 값들중 8개의 파라미터는 My_param에 넣고 Key는 My_sk에 넣어준다.



/// 시연할려문 IMPORT,EXPORT 코드에서 IP부분 바꿔야 할거같은데 그건 아직 안 건들였어
/// 참고로 IMPORT,EXPORT는 KGC,DNS에 상관없이 똑같은거 써도돼