#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define IP_ADDRESS "fdc7:9dd5:2c66:be86:7e57:58ff:fe68:aea9"

int main(int argc, char *argv[]) {
    int sock = socket(AF_INET6, SOCK_DGRAM, 0);

    struct sockaddr_in6 addrA = {AF_INET6, htons(7777), 0, 0, 0};
    inet_pton(AF_INET6, IP_ADDRESS, (struct sockaddr*)&addrA.sin6_addr);

    sendto(sock, argv[1], strlen(argv[1]), 0, (struct sockaddr*)&addrA, sizeof(addrA));

    uint8_t buf[8];
    recv(sock, buf, 8, 0);

    if(memcmp("OK", buf, 2) == 0) {
        close(sock);
        printf("KOK\n");
        return 0;
    } 
    
    uint16_t tm;
    uint32_t nb;
    memcpy(&tm, buf+2, 2);
    memcpy(&nb, buf+4, 4);
    nb = ntohl(nb);
    tm = ntohs(tm);

    if(cree_fic(argv[1]) == 0){
        char fic[tm+1];
        for(int i = 0; i < nb; i++) {
            int lu = recv(sock, buf, tm, 0);
            if(lu == -1) {
                close(sock);
                return EXIT_FAILURE;
            }
            fic[lu] = '\0';
            texte_append(argv[1], fic, i, lu);
        }
    }

    close(sock);
    return 0;
}