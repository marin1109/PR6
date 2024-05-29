#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <poll.h>

#define SIZE_MESS 100

int main(int argc, char *args[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", args[0]);
        exit(1);
    }

    int sock = socket(PF_INET6, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("creation socket");
        exit(1);
    }

    struct sockaddr_in6 address_sock;
    memset(&address_sock, 0, sizeof(address_sock));
    address_sock.sin6_family = AF_INET6;
    address_sock.sin6_port = htons(atoi(args[1]));
    address_sock.sin6_addr = in6addr_any;

    int r = bind(sock, (struct sockaddr *)&address_sock, sizeof(address_sock));
    if (r < 0)
    {
        perror("erreur bind");
        exit(2);
    }

    r = listen(sock, 0);
    if (r < 0)
    {
        perror("erreur listen");
        exit(2);
    }

    int listener = sock;
    int fd_size = 5;
    struct pollfd *pfds = malloc(sizeof(*pfds) * fd_size);
    memset(pfds, 0, sizeof(*pfds) * fd_size);
    pfds[0].fd = listener;
    pfds[0].events = POLLIN;

    int sockclient, fd_count = 1;
    char buf[SIZE_MESS];

    while (1)
    {
        int poll_cpt = poll(pfds, fd_count, 10000);

        if (poll_cpt == -1)
        {
            perror("poll");
            exit(1);
        }

        if (poll_cpt == 0)
        {
            printf("je m'ennuie...\n");
            continue;
        }

        for (int i = 0; i < fd_count; i++)
        {
            if (pfds[i].revents & POLLIN)
            {
                if (pfds[i].fd == listener)
                {
                    if (fd_count >= fd_size)
                        continue;

                    sockclient = accept(listener, NULL, NULL);
                    if (sockclient == -1)
                    {
                        perror("accept");
                    }
                    else
                    {
                        pfds[fd_count].fd = sockclient;
                        pfds[fd_count].events = POLLIN;
                        fd_count++;
                        printf("pollserver: nouvelle connexion sur la socket %d\n", sockclient);
                    }
                }
                else
                {
                    int recu = recv(pfds[i].fd, buf, sizeof(buf) - 1, 0);
                    if (recu <= 0)
                    {
                        if (recu == 0)
                        {
                            printf("pollserver: socket %d closed\n", pfds[i].fd);
                        }
                        else
                        {
                            perror("recv");
                        }
                        close(pfds[i].fd);
                        pfds[i] = pfds[fd_count - 1];
                        fd_count--;
                    }
                    else
                    {
                        buf[recu] = '\0';
                        printf("recu sur socket %d : %s", pfds[i].fd, buf);
                    }
                }
            }
        }
    }

    return 0;
}
