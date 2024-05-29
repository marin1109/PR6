#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

int main(int argc, char const *argv[])
{
    int sock;

    if (argc < 2)
    {
        fprintf(stderr, "usage: ./dif4 <message>\n");
        exit(EXIT_FAILURE);
    }

    /* créer la socket */
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("erreur socket");
        return 1;
    }

    /* Initialisation de l'adresse d'abonnement */
    struct sockaddr_in gradr;
    memset(&gradr, 0, sizeof(gradr));
    gradr.sin_family = AF_INET;
    inet_pton(AF_INET, "225.1.2.3", &gradr.sin_addr);
    gradr.sin_port = htons(4321);

    /* Initialisation de l'interface */
    int ifindex = if_nametoindex("eth0");
    if (ifindex == 0)
        perror("if_nametoindex");

    struct ip_mreqn group;
    memset(&group, 0, sizeof(group));
    group.imr_multiaddr.s_addr = htonl(INADDR_ANY);
    group.imr_ifindex = ifindex;

    if (    )
    {
        perror("erreur initialisation de l'interface locale");
        return 1;
    }

    if (sendto(sock, argv[1], strlen(argv[1]), 0, (struct sockaddr *)&gradr, sizeof(gradr)) < 0)
        printf("erreur send\n");

    close(sock);
    return 0;
}
