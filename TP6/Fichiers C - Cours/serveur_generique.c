#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#ifdef MAC
#ifdef SO_REUSEADDR
#undef SO_REUSEADDR
#endif
#define SO_REUSEADDR SO_REUSEPORT
#endif

#define SIZE_MESS 100
#define MESS "Quand on ne sait pas où l'on va, il faut aller vite, et le plus vite possible."

void affiche_adresse(struct sockaddr_in6 adr)
{
    char adr_buf[INET6_ADDRSTRLEN];
    memset(adr_buf, 0, sizeof(adr_buf));

    inet_ntop(AF_INET6, &(adr.sin6_addr), adr_buf, sizeof(adr_buf));
    printf("adresse client : IP: %s port: %d\n", adr_buf, ntohs(adr.sin6_port));

    char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
    if (getnameinfo((struct sockaddr *)&adr, sizeof(adr), hbuf, sizeof(hbuf), sbuf,
                    sizeof(sbuf), 0) == 0)
        printf("client = %s, service = %s\n", hbuf, sbuf);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "usage : ./serveur_generique <PORT>\n");
        exit(1);
    }

    //*** creation de l'adresse du destinataire (serveur) ***
    struct sockaddr_in6 address_sock;
    address_sock.sin6_family = AF_INET6;
    address_sock.sin6_port = htons(atoi(argv[1]));
    address_sock.sin6_addr = in6addr_any;

    //*** creation de la socket ***
    int sock = socket(PF_INET6, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("creation socket");
        exit(1);
    }

    //*** desactiver l'option n'accepter que de l'IPv6 **
    int optval = 0;
    int r = setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, &optval, sizeof(optval));
    if (r < 0)
        perror("erreur connexion IPv4 impossible");

    //*** le numero de port peut etre utilise en parallele ***
    optval = 1;
    r = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (r < 0)
        perror("erreur réutilisation de port impossible");

    //*** on lie la socket au port ***
    r = bind(sock, (struct sockaddr *)&address_sock, sizeof(address_sock));
    if (r < 0)
    {
        perror("erreur bind");
        exit(2);
    }

    //*** Le serveur est pret a ecouter les connexions sur le port ***
    r = listen(sock, 0);
    if (r < 0)
    {
        perror("erreur listen");
        exit(2);
    }

    //*** le serveur accepte une connexion et cree la socket de communication avec le client ***
    struct sockaddr_in6 adrclient;
    memset(&adrclient, 0, sizeof(adrclient));
    socklen_t size = sizeof(adrclient);
    int sockclient = accept(sock, (struct sockaddr *)&adrclient, &size);

    if (sockclient >= 0)
    {

        affiche_adresse(adrclient);

        //*** reception d'un message ***
        char buf[SIZE_MESS];
        memset(buf, 0, SIZE_MESS);
        int recu = recv(sockclient, buf, (SIZE_MESS - 1) * sizeof(char), 0);
        if (recu <= 0)
        {
            perror("erreur lecture");
            exit(4);
        }
        printf("%s\n", buf);

        //*** envoie d'un message ***
        int ecrit = send(sockclient, MESS, strlen(MESS), 0);
        if (ecrit <= 0)
        {
            perror("erreur ecriture");
            exit(3);
        }
    }

    //*** fermeture socket client ***
    close(sockclient);

    //*** fermeture socket serveur ***
    close(sock);

    return 0;
}
