#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define IF_NAMETOINDEX "wlo1"
#define PORT 12121
#define GROUP "ff12::1"

int main(int argc, char const *argv[])
{
    int sock;

    if (argc < 2)
    {
        fprintf(stderr, "usage: ./dif6 <message>\n");
        exit(EXIT_FAILURE);
    }

    /* créer la socket */
    if ((sock = socket(AF_INET6, SOCK_DGRAM, 0)) < 0)
    {
        perror("erreur socket");
        return 1;
    }

    /* Initialisation de l'adresse d'abonnement */
    struct sockaddr_in6 gradr;
    memset(&gradr, 0, sizeof(gradr));
    gradr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, GROUP, &gradr.sin6_addr);
    gradr.sin6_port = htons(PORT);

    int ifindex = if_nametoindex(IF_NAMETOINDEX);
    if (ifindex == 0)
        perror("if_nametoindex");

    gradr.sin6_scope_id = ifindex; // ou 0 pour interface par défaut

    if (sendto(sock, argv[1], strlen(argv[1]), 0, (struct sockaddr *)&gradr, sizeof(gradr)) < 0)
        printf("erreur send\n");

    close(sock);
    return 0;
}
