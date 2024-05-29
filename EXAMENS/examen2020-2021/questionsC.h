#ifndef QUESTIONSC_H
#define QUESTIONSC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


// question 1
int question1(){
    struct addrinfo hints, *res, *p;
    int status, sockfd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if((status = getaddrinfo("machine1.progreseaux.fr", "5678", &hints, &res)) != 0){
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(1);
    }
    p = res;
    while(p != NULL){
        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) != -1){
            if(connect(sockfd, p->ai_addr, p->ai_addrlen) != -1){
                break;
            } else
                close(sockfd);
        }
        p = p->ai_next;
    }
    if(p == NULL){
        freeaddrinfo(res);
        return 2;
    }
    char *msg = "Hello World!\n";
    if( send(sockfd, msg, strlen(msg), 0) == -1){
        freeaddrinfo(res);
        return 3;
    }
    freeaddrinfo(res);
    return sockfd;
}

// question 2
void requestLeaf(){
    int sockfd = question1();
    if(sockfd == 1){
        return;
    }
    char *msg = "LEAF\n";
    if(send(sockfd, msg, strlen(msg), 0) == -1){
        return;
    }
    close(sockfd);
}

char* getLeaf(){
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd == -1){
        return NULL;
    }
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(4242);
    server.sin_addr.s_addr = INADDR_ANY;
    if(bind(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1){
        return NULL;
    }

    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr("233.222.222.1");
    mreq.imr_interface.s_addr = INADDR_ANY;
    if(setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) == -1){
        return NULL;
    }
    char buf[21];
    if(recv(sockfd, buf, 20, 0) == -1){
        return NULL;
    }
    buf[20] = '\0';
    return buf;
}

int nextLeaf(char *h) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        return 1;
    }
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(5678);
    inet_aton(h, &server.sin_addr);
    if(connect(sockfd, (struct sockaddr *)&server, sizeof(server)) == -1){
        return 1;
    }
    char *sendmess = "CONNECT\n";
    if(send(sockfd, sendmess, strlen(sendmess), 0) == -1){
        return 1;
    }
    char buf[7];
    if(recv(sockfd, buf, 6, 0) == -1){
        return 1;
    }
    buf[6] = '\0';
    if(strcmp(buf, "OKSON\n") != 0){
        close(sockfd);
        return 0;
    }
    close(sockfd);
    return -1;
}

int main(int argc, char *argv[]){
    int found = 0;
    requestLeaf();
    char *h = getLeaf();
    while(!found){
        found = nextLeaf(h);
    }
    printf("I am leaf\n");
}

#endif