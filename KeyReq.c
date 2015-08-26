#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
	
#define BUF_SIZE 4096
#define NAME_SIZE 20
	
void recv_param();
int recvn(int s, char *buf, int len);
void error_handling(char * msg);

char msg[BUF_SIZE];
char param[100];

	
int main(int argc, char **argv)
{
	int sock;
	struct sockaddr_in serv_addr;

	if(argc!=4) {
		printf("Usage : %s <Target_IP> <URL> <IP>\n", argv[0]);
		exit(1);
	}

	strcpy(param, argv[2]);
	strcat(param, "^"); strcat(param, argv[3]);

	sock = socket(PF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_addr.sin_port=htons(5959);
	  
	if( connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		error_handling("connect() error");

	if(write(sock, param, strlen(param)) == -1) {
		error_handling("param write() error");
	}
	close(sock);
	printf("parameters Sent : %s\n", param);

	recv_param();

	return 0;
}

void recv_param() {
	int retval;
    int listen_sock = socket(PF_INET, SOCK_STREAM, 0);
    char filename[256];
    
    if(listen_sock == -1)
        error_handling("socket() error");
 
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(5960);
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
    char buf[BUF_SIZE];

    printf("Waiting for Parameters..\n");
 
    while(1) {
        addrlen = sizeof(clientaddr);
        client_sock = accept(listen_sock, (struct sockaddr*) &clientaddr, &addrlen);
        if(client_sock == -1)
            error_handling("accept() error");
 		if(filename[0] == '\0')
        	printf("\n->FileSender connect : IP = %s, Port = %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
 
        memset(filename, 0, sizeof(filename));
        retval = recvn(client_sock, filename, 256);
        if(retval == -1) {
            error_handling("recv() error");
            close(client_sock);
            continue;
        }
        if(strcmp(filename, "END") == 0) {
        	printf("File Transfer END\n");
        	close(client_sock);
        	break;
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
            retval = recvn(client_sock, buf, BUF_SIZE);
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
        printf("FileSender complete : IP = %s, Port = %d\n\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
 
    }
    close(listen_sock);
}

int recvn(int s, char *buf, int len) {
    int received;
    char *ptr = buf;
    int left = len;
 
    while(left > 0) {
        received = read(s, ptr, left);
        if(received == -1)
            return -1;
        else if(received == 0)
            break;
        left -= received;
        ptr += received;
    }
 
    return (len - left);
}
	
void error_handling(char *msg)
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
