#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define SIZE_MESS 100
#define MESS "La culture c'est ce qu'il reste quand on a tout oublié."

void affiche_adresse(struct sockaddr adr, int adrlen)
{
    char adr_buf[INET6_ADDRSTRLEN];
    if (adrlen == sizeof(struct sockaddr_in))
    {
        struct sockaddr_in adr4 = *((struct sockaddr_in *)&adr);

        inet_ntop(AF_INET, &(adr4.sin_addr), adr_buf, sizeof(adr_buf));
        printf("adresse serveur : IP: %s port: %d\n", adr_buf, ntohs(adr4.sin_port));
    }
    else
    {
        struct sockaddr_in6 adr6 = *((struct sockaddr_in6 *)&adr);

        inet_ntop(AF_INET6, &(adr6.sin6_addr), adr_buf, sizeof(adr_buf));
        printf("adresse serveur : IP: %s port: %d\n", adr_buf, ntohs(adr6.sin6_port));
    }
}

int get_server_addr(char *hostname, char *port, int *sock, struct sockaddr *addr, int *addrlen)
{
    struct addrinfo hints, *r, *p;
    int ret;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((ret = getaddrinfo(hostname, port, &hints, &r)))
    {
        fprintf(stderr, "erreur getaddrinfo : %s\n", gai_strerror(ret));
        return -1;
    }

    p = r;
    while (p != NULL)
    {
        if ((*sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) > 0)
        {
            if (connect(*sock, p->ai_addr, p->ai_addrlen) == 0)
                break;
            printf("connexion impossible sur : %d %d\n", p->ai_addrlen, sizeof(struct sockaddr_in));
            affiche_adresse(*(p->ai_addr), p->ai_addrlen);
            close(*sock);
        }

        p = p->ai_next;
    }

    if (p == NULL)
        return -2;

    // on stocke l'adresse de connexion
    *addrlen = p->ai_addrlen;
    if (p->ai_family == AF_INET)
        memcpy(addr, (struct sockaddr_in *)p->ai_addr, p->ai_addrlen);
    else
        memcpy(addr, (struct sockaddr_in6 *)p->ai_addr, p->ai_addrlen);

    // on libère la mémoire allouée par getaddrinfo
    freeaddrinfo(r);

    return 0;
}

int main(int argc, char **args)
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <hostname> <port>\n", args[0]);
        exit(1);
    }

    struct sockaddr server_addr;
    int fdsock, adrlen;

    switch (get_server_addr(args[1], args[2], &fdsock, &server_addr, &adrlen))
    {
    case 0:
        printf("adresse creee !\n");
        break;
    case -1:
        fprintf(stderr, "Erreur: hote non trouve.\n");
        exit(1);
    case -2:
        fprintf(stderr, "Erreur: echec de creation de la socket.\n");
        exit(1);
    }

    affiche_adresse(server_addr, adrlen);

    //*** envoie d'un message ***
    int ecrit = send(fdsock, MESS, strlen(MESS), 0);
    if (ecrit <= 0)
    {
        perror("erreur ecriture");
        exit(3);
    }

    //*** reception d'un message ***
    char buf[SIZE_MESS + 1];
    memset(buf, 0, SIZE_MESS + 1);
    int recu = recv(fdsock, buf, SIZE_MESS, 0);
    if (recu <= 0)
    {
        perror("erreur lecture");
        exit(4);
    }
    printf("%s\n", buf);

    close(fdsock);

    return 0;
}
