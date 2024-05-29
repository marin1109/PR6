#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <poll.h>

#define BUF_SIZE 1024
#define TAILLE_MAX 512

int fic_exist(char* path) {
    FILE* fic = fopen(path, "r");
    if(fic == NULL) {
        return 0;
    }
    fclose(fic);
    return 1;
}
int fic_taille(char* path) {
    FILE* fic = fopen(path, "r");
    fseek(fic, 0, SEEK_END);
    int taille = ftell(fic);
    fclose(fic);
    return taille;
}

void mess_num(int nb, char* mess);

int main(int argc, char* argv[]) {
    int sock = socket(AF_INET6, SOCK_DGRAM, 0);
    struct sockaddr_in6 addrB = {AF_INET6, htons(7777), 0, IN6ADDR_ANY_INIT, 0};
    bind(sock, (struct sockaddr*)&addrB, sizeof(addrB));

    char buf[BUF_SIZE];
    struct sockaddr_in6 addrA;
    socklen_t addrA_len = sizeof(addrA);
    int lu = recvfrom(sock, buf, BUF_SIZE, 0, (struct sockaddr*)&addrA, &addrA_len);
    buf[lu] = '\0';
    if(!fic_exist(buf)) {
        sendto(sock, "NOK", 3, 0, (struct sockaddr*)&addrA, addrA_len);
        close(sock);
        return EXIT_FAILURE;
    }

    char mess[8];

    memcpy(mess, "OK", 2);

    uint16_t tm = htons(TAILLE_MAX);
    memcpy(mess+2, &tm, 2);

    uint32_t nb = fic_taille(buf)/TAILLE_MAX+fic_taille(buf)%TAILLE_MAX;
    memcpy(mess+4, "%d", 4);

    sendto(sock, mess, 8, 0, (struct sockaddr*)&addrA, addrA_len);

    struct pollfd fds[1];
    fds[0].fd = sock;
    fds[0].events = POLLIN|POLLOUT;
    int timeout = -1;
    int cpt = 0;
    char fic[TAILLE_MAX+1];

    while(1){
        int val = poll(fds, 1, timeout);
        if(fds[0].revents & POLLOUT) {
            int lu = lire_fic(buf, fic, cpt, TAILLE_MAX);
            mess_num(cpt, mess);
            sendto(sock, fic, lu, 0, (struct sockaddr*)&addrA, addrA_len);
            cpt++;
        }
        if(fds[0].revents & POLLIN) {
            int nbb;
            recvfrom(sock, &nbb, sizeof(nb), 0, (struct sockaddr*)&addrA, &addrA_len);
            nbb = ntohl(nbb);
            int lu = lire_fic(buf, fic, nb, TAILLE_MAX);
            mess_num(nbb, mess);
            sendto(sock, fic, lu, 0, (struct sockaddr*)&addrA, addrA_len);
        }
        if(cpt == nb) {
            timeout = 5000; // attendre 5 secondes à la fin de toutes les transmissions pour être 
                            // sûr qu'on a bien reçu toutes les demandes de retransmission
            fds[0].events = POLLIN; // on ne cherche plus qu'à envoyer les fichiers redemandés 
        }
        if(val == 0) {
            break;
        }
    }

    close(sock);
    return EXIT_SUCCESS;
}

void mess_num(int nb, char* mess) {
    nb = htonl(nb);
    char *tmp = realloc(mess, 4);
    memmove(tmp+4, &nb, 4);
}