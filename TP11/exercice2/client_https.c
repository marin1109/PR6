#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netdb.h>
#include <unistd.h>

#define PORT 443
#define BUFFER_SIZE 4096
#define BUFFER_CHAINE 256

int get_server_addr(const char *hostname, int *sockfd, struct sockaddr_in *serv_addr) {
    struct hostent *host;

    host = gethostbyname(hostname);
    if (!host) {
        perror("gethostbyname() failed");
        return -1;
    }

    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (*sockfd < 0) {
        perror("socket() failed");
        return -1;
    }

    memset(serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_port = htons(PORT);
    memcpy(&serv_addr->sin_addr.s_addr, host->h_addr, host->h_length);

    if (connect(*sockfd, (struct sockaddr *)serv_addr, sizeof(struct sockaddr_in)) < 0) {
        perror("connect() failed");
        close(*sockfd);
        return -2;
    }

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hostname>\n", argv[0]);
        return EXIT_FAILURE;
    }

    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
        fprintf(stderr, "SSL_CTX_new() failed.\n");
        ERR_print_errors_fp(stderr);
        return EXIT_FAILURE;
    }

    char* hostname = argv[1];
    int sockfd;
    struct sockaddr_in serv_addr;

    if (get_server_addr(hostname, &sockfd, &serv_addr) < 0) {
        SSL_CTX_free(ctx);
        return EXIT_FAILURE;
    }

    SSL *ssl = SSL_new(ctx);
    if (!ssl) {
        fprintf(stderr, "SSL_new() failed.\n");
        ERR_print_errors_fp(stderr);
        close(sockfd);
        SSL_CTX_free(ctx);
        return EXIT_FAILURE;
    }

    SSL_set_fd(ssl, sockfd);
    if (SSL_connect(ssl) != 1) {
        fprintf(stderr, "SSL_connect() failed.\n");
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);
        return EXIT_FAILURE;
    }

    char chaine[BUFFER_CHAINE];
    while (1) {
        printf("? ");
        if (fgets(chaine, sizeof(chaine), stdin) == NULL) {
            break;
        }

        ssize_t len = strlen(chaine);
        if (len > 0 && chaine[len - 1] == '\n') {
            chaine[len - 1] = '\0';
        }

        if (strcmp(chaine, "quit") == 0) {
            break;
        }

        char request[BUFFER_SIZE];
        snprintf(request, sizeof(request), "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\n\r\n", chaine, hostname);

        if (SSL_write(ssl, request, strlen(request)) <= 0) {
            fprintf(stderr, "SSL_write() failed.\n");
            ERR_print_errors_fp(stderr);
            SSL_shutdown(ssl);
            SSL_free(ssl);
            close(sockfd);
            SSL_CTX_free(ctx);
            return EXIT_FAILURE;
        }

        char response[BUFFER_SIZE];
        int bytes, total_bytes = 0;
        char *header_end;
        int header_received = 0;

        while ((bytes = SSL_read(ssl, response + total_bytes, sizeof(response) - total_bytes - 1)) > 0) {
            total_bytes += bytes;
            response[total_bytes] = '\0';

            if (!header_received) {
                header_end = strstr(response, "\r\n\r\n");
                if (header_end) {
                    header_received = 1;
                    printf("%.*s", (int)(header_end - response + 4), response); // Affichage de l'entête
                    memmove(response, header_end + 4, total_bytes - (header_end - response + 4));
                    total_bytes -= (header_end - response + 4);
                }
            }

            if (header_received) {
                printf("%s", response);
                total_bytes = 0;
            }
        }

        if (bytes < 0) {
            fprintf(stderr, "SSL_read() failed.\n");
            ERR_print_errors_fp(stderr);
        }
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sockfd);
    SSL_CTX_free(ctx);
    EVP_cleanup();

    return EXIT_SUCCESS;
}
