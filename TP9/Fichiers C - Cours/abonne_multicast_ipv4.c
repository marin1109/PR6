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

    /* créer la socket */
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("echec de socket");
        return 1;
    }

    /* SO_REUSEADDR permet d'avoir plusieurs instances de cette application      */
    /* ecoutant sur le port multicast et recevant chacune les differents paquets */
    int ok = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof(ok)) < 0)
    {
        perror("echec de SO_REUSEADDR");
        close(sock);
        return 1;
    }

    /* Initialisation de l'adresse de reception */
    struct sockaddr_in adr;
    memset(&adr, 0, sizeof(adr));
    adr.sin_family = AF_INET;
    adr.sin_addr.s_addr = htonl(INADDR_ANY);
    adr.sin_port = htons(4321);

    if (bind(sock, (struct sockaddr *)&adr, sizeof(adr)))
    {
        perror("echec de bind");
        close(sock);
        return 1;
    }

    int ifindex = if_nametoindex("eth0");
    if (ifindex == 0)
        perror("if_nametoindex");

    struct ip_mreqn group;
    memset(&group, 0, sizeof(group));
    group.imr_multiaddr.s_addr = inet_addr("225.1.2.3");
    group.imr_ifindex = ifindex;

    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &group, sizeof(group)) < 0)
    {
        perror("echec de abonnement groupe");
        close(sock);
        return 1;
    }

    char buf[10];
    memset(buf, 0, sizeof(buf));
    if (read(sock, buf, sizeof(buf) - 1) < 0)
    {
        perror("echec de read");
        return -1;
    }
    printf("Message : %s\n", buf);

    close(sock);
    return 0;
}
