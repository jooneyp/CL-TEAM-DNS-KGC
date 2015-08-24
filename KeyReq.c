#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
	
#define BUF_SIZE 4096
#define NAME_SIZE 20
	
void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_handling(char * msg);
char msg[BUF_SIZE];
char param[100];
	
int main(int argc, char **argv)
{
	int sock;
	struct sockaddr_in serv_addr;
	pthread_t snd_thread, rcv_thread;
	void * thread_return;
	if(argc!=4) {
		printf("Usage : %s <Target_IP> <IP> <URL>\n", argv[0]);
		exit(1);
	}

	strcpy(param, argv[2]);
	strcat(param, "^"); strcat(param, argv[3]);
	printf("%s\n", param);

	sock = socket(PF_INET, SOCK_STREAM, 0);
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(5959);
	  
	if( connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error");
	
	pthread_create(&snd_thread, NULL, send_param, (void*)&sock);
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock);
	pthread_join(snd_thread, &thread_return);
	pthread_join(rcv_thread, &thread_return);
	close(sock);  
	return 0;
}
	
void * send_param(void * arg)   // send thread main
{
	int sock = *((int*)arg);
	write(sock, param, strlen(param));
	return NULL;
}

void * recv_msg(void * arg)   // read thread main
{
    int retval;
    int listen_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (argc!=2) {
     printf("Usage: %s <port>\n",argv[0]);
    }

    if(listen_sock == -1)
        error_handling("socket() error");
 
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(atoi(argv[1]));
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    retval = bind(listen_sock, (struct sockaddr*) &serveraddr, sizeof(serveraddr));
    if(retval == -1)
        error_handling("bind() error");
 
    retval = listen(listen_sock, 5);
    if(retval == -1)
        error_handling("listen() error");
 
    int client_sock;
    struct sockaddr_in clientaddr;
    int addrlen;
    char buf[BUFSIZE];
 
    while(1) {
        addrlen = sizeof(clientaddr);
        client_sock = accept(listen_sock, (struct sockaddr*) &clientaddr, &addrlen);
        if(client_sock == -1)
            error_handling("accept() error");
 
        printf("\n->FileSender connect : IP = %s, Port = %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
 
        char filename[256];
        memset(filename, 0, sizeof(filename));
        retval = recvn(client_sock, filename, 256);
        if(retval == -1) {
            error_handling("recv() error");
            close(client_sock);
            continue;
        }
        printf("-> filename : %s\n", filename);
 
        int totalbytes;
        retval = recvn(client_sock, (char *) &totalbytes, sizeof(totalbytes));
        if(retval == -1) {
            error_handling("recv() error");
            close(client_sock);
            continue;
        }
        printf("-> filesize : %d\n", totalbytes);
 
        FILE *fp = fopen(filename, "wb");
        if(fp == NULL) {
            error_handling("File I/O error");
            close(client_sock);
            continue;
        }
 
        int numtotal = 0;
        while(1) {
            retval = recvn(client_sock, buf, BUFSIZE);
            if(retval == -1) {
                error_handling("recv() error");
                break;
            }
            else if(retval == 0)
                break;
            else {
                fwrite(buf, 1, retval, fp);
                numtotal += retval;
            }
        }
        fclose(fp);
 
        if(numtotal == totalbytes)
            printf("-> File trans complete\n");
        else
            printf("-> File trans failed\n");
 
        close(client_sock);
        printf("FileSender complete : IP = %s, Port = %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
 
    }
 
    close(listen_sock);
 
    return NULL;
}
	
void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
