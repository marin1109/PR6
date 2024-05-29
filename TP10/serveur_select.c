#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

#define BUFFER_SIZE 512

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    struct sockaddr_in addr;
    int sock_serv, sock_client, max_fd, activity;
    socklen_t addrlen;
    char msg[BUFFER_SIZE];
    fd_set a_surveiller;
    fd_set activite;

    sock_serv = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_serv == -1) {
        perror("socket");
        return EXIT_FAILURE;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[1]));
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock_serv, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(sock_serv);
        return EXIT_FAILURE;
    }

    if (listen(sock_serv, SOMAXCONN) == -1) {
        perror("listen");
        close(sock_serv);
        return EXIT_FAILURE;
    }

    FD_ZERO(&a_surveiller);
    FD_SET(sock_serv, &a_surveiller);
    max_fd = sock_serv;

    while (1) {
        activite = a_surveiller;

        activity = select(max_fd + 1, &activite, NULL, NULL, NULL);
        if (activity < 0 && errno != EINTR) {
            perror("select");
            break;
        }

        if (FD_ISSET(sock_serv, &activite)) {
            addrlen = sizeof(addr);
            sock_client = accept(sock_serv, (struct sockaddr*)&addr, &addrlen);
            if (sock_client < 0) {
                perror("accept");
                continue;
            }

            FD_SET(sock_client, &a_surveiller);
            if (sock_client > max_fd) {
                max_fd = sock_client;
            }

            printf("Nouveau client connecté, socket fd %d, ip %s, port %d\n", 
                   sock_client, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        }

        for (int i = 0; i <= max_fd; i++) {
            if (i != sock_serv && FD_ISSET(i, &activite)) {
                int valread = read(i, msg, BUFFER_SIZE);
                if (valread == 0) {
                    getpeername(i, (struct sockaddr*)&addr, &addrlen);
                    printf("Client déconnecté, ip %s, port %d\n", 
                           inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

                    close(i);
                    FD_CLR(i, &a_surveiller);
                } else {
                    msg[valread] = '\0';
                    send(i, msg, valread, 0);
                }
            }
        }
    }

    close(sock_serv);
    return EXIT_SUCCESS;
}
