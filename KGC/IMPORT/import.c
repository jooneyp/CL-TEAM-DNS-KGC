#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
 
#define BUFSIZE 4096
 
void err_quit(char *msg) {
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
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
 
 //TCP/IP FTP(File Transfer Protocol)서버 부분(파일 수신만) 
int main(int argc, char *argv[])
{
    int retval;
 
    int listen_sock = socket(PF_INET, SOCK_STREAM, 0);

    if(listen_sock == -1)
        err_quit("socket() error");
 
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(9000);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    retval = bind(listen_sock, (struct sockaddr*) &serveraddr, sizeof(serveraddr));
    if(retval == -1)
        err_quit("bind() error");
 
    retval = listen(listen_sock, 5);
    if(retval == -1)
        err_quit("listen() error");
 
    int client_sock;
    struct sockaddr_in clientaddr;
    int addrlen;
    char buf[BUFSIZE];
 
    while(1) {
        addrlen = sizeof(clientaddr);
        client_sock = accept(listen_sock, (struct sockaddr*) &clientaddr, &addrlen);
        if(client_sock == -1)
            err_quit("accept() error");
 
        printf("\n->FileSender connect : IP = %s, Port = %d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
 
        char filename[256];
        memset(filename, 0, sizeof(filename));
        retval = recvn(client_sock, filename, 256);
        if(retval == -1) {
            err_quit("recv() error");
            close(client_sock);
            continue;
        }
        printf("-> filename : %s\n", filename);
 
        int totalbytes;
        retval = recvn(client_sock, (char *) &totalbytes, sizeof(totalbytes));
        if(retval == -1) {
            err_quit("recv() error");
            close(client_sock);
            continue;
        }
        printf("-> filesize : %d\n", totalbytes);
 
        FILE *fp = fopen(filename, "wb");
        if(fp == NULL) {
            err_quit("File I/O error");
            close(client_sock);
            continue;
        }
 
        int numtotal = 0;
        while(1) {
            retval = recvn(client_sock, buf, BUFSIZE);
            if(retval == -1) {
                err_quit("recv() error");
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
 
    return 0;
 
}