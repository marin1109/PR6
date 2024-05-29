#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>


int main(){
    int sock2 = socket(AF_INET, SOCK_STREAM, 0);
    if (sock2 == -1) {
        perror("socket");
        return 2;
    }
    if(setsockopt(sock2, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) == -1){
        perror("setsockopt");
        return 2;
    }
    struct sockaddr_in addr2;
    addr2.sin_family = AF_INET;
    addr2.sin_port = htons(8080);
    addr2.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (bind(sock2, (struct sockaddr *)&addr2, sizeof(addr2)) == -1) {
        perror("bind");
        return 2;
    }
    if (listen(sock2, 5) == -1) {
        perror("listen");
        return 2;
    }
    int sock = accept(sock2, NULL, NULL);
    uint32_t len;
    ssize_t n;
    n = recv(sock, &len, sizeof(len), 0);
    if (n == -1 || n != sizeof(len)) {
        perror("recv1");
        return 2;
    }
    len = ntohl(len);
    char *buffer = malloc(len + 1);
    if (buffer == NULL) {
        perror("malloc");
        return 2;
    }
    if (recv(sock, buffer, len, 0) == -1) {
        perror("recv2");
        free(buffer);
        return 2;
    }
    buffer[len] = '\0';
    printf("%s\n", buffer);
    free(buffer);
    close(sock);
    return 0;
}
