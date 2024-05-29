#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

int main(int argc, char* argv[]){
    if(argc != 2){
        fprintf(stderr, "Usage: %s <domain>\n", argv[0]);
        return EXIT_FAILURE;
    }

    srand(time(NULL));

    uint16_t header[6] = {0};
    u_int16_t id = rand() % 65535;
    u_int16_t qr = 0;
    u_int16_t opcode = 0;
    u_int16_t aa = 0;
    u_int16_t tc = 0;
    u_int16_t rd = 1;
    u_int16_t ra = 0;
    u_int16_t z = 0;
    u_int16_t rcode = 0;

    header[0] = htons(id); // ID
    header[1] = htons((qr << 15) | (opcode << 11) | (aa << 10) | (tc << 9) | (rd << 8) | (ra << 7) | (z << 4) | rcode); // Flags
    header[2] = htons(1); // QDCOUNT
    header[3] = htons(0); // ANCOUNT
    header[4] = htons(0); // NSCOUNT
    header[5] = htons(0); // ARCOUNT

    int fic = open("requete_dns", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fic == -1)
    {
        perror("open");
        return EXIT_FAILURE;
    }

    write(fic, header, sizeof(header));
    char *domain = argv[1];
    size_t domain_len = strlen(domain);
    char *qname = malloc(domain_len + 2); // +2 pour l'octet de longueur initial et l'octet de terminaison 0
    if (qname == NULL) {
        perror("malloc");
        return EXIT_FAILURE;
    }

    char *q = qname;
    char *d = domain;

    while (*d) {
        char *start = d;
        while (*d && *d != '.') {
            d++;
        }
        int segment_len = d - start;
        *q++ = segment_len;
        memcpy(q, start, segment_len);
        q += segment_len;

        if (*d == '.') {
            d++;
        }
    }

    *q = '\0'; // Terminaison par 0

    // Écriture de QNAME dans le fichier
    write(fic, qname, q - qname + 1); // +1 pour inclure l'octet nul terminal
    printf("%s\n", qname);

    u_int16_t qtype = htons(1);
    u_int16_t qclass = htons(1);
    write(fic, &qtype, sizeof(u_int16_t));
    write(fic, &qclass, sizeof(u_int16_t));

    close(fic);
    return EXIT_SUCCESS;
}