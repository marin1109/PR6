#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define DNS_PORT 53
#define DNS_SERVER_IP "127.0.0.53"

// Fonction pour lire un nom de domaine (avec gestion de la compression)
char* read_name(unsigned char* reader, unsigned char* buffer, int* count) {
    unsigned char *name;
    int p = 0, jumped = 0, offset;
    *count = 1;
    name = (unsigned char*)malloc(256);

    name[0] = '\0';

    // Lire le nom
    while (*reader != 0) {
        // Vérifier la compression
        if (*reader >= 192) {
            offset = (*reader) * 256 + *(reader + 1) - 49152; // 49152 = 192 * 256
            reader = buffer + offset - 1;
            jumped = 1; // Nous avons sauté en utilisant la compression donc pas d'incrément de count
        } else {
            name[p++] = *reader;
        }
        reader = reader + 1;

        if (jumped == 0) {
            *count = *count + 1;
        }
    }

    name[p] = '\0'; // Terminer la chaîne

    if (jumped == 1) {
        *count = *count + 1; // Compter les 2 octets du pointeur
    }

    // Convertir la chaîne en nom de domaine
    for (int i = 0; i < (int)strlen((const char*)name); i++) {
        p = name[i];
        for (int j = 0; j < (int)p; j++) {
            name[i] = name[i + 1];
            i = i + 1;
        }
        name[i] = '.';
    }
    name[strlen((const char*)name) - 1] = '\0'; // Supprimer le dernier point

    return (char*)name;
}

// Fonction pour construire une question DNS
void build_question(char *domain, uint16_t qtype, unsigned char **qname, size_t *qname_len) {
    size_t domain_len = strlen(domain);
    *qname = malloc(domain_len + 2); // +2 pour l'octet de longueur initial et l'octet de terminaison 0
    if (*qname == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    unsigned char *q = *qname;
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

    *qname_len = (q - *qname + 1) + sizeof(qtype) + sizeof(uint16_t); // +1 pour inclure l'octet nul terminal

    // Allouer l'espace pour la question complète (QNAME + QTYPE + QCLASS)
    *qname = realloc(*qname, *qname_len);
    if (*qname == NULL) {
        perror("realloc");
        exit(EXIT_FAILURE);
    }

    // Ajouter QTYPE et QCLASS
    q = *qname + (q - *qname + 1); // Pointer à la fin de QNAME
    *(uint16_t *)q = htons(qtype);
    q += sizeof(uint16_t);
    *(uint16_t *)q = htons(1); // QCLASS: IN
}

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <domain> [A|AAAA|BOTH]\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *domain = argv[1];
    char *query_type = argc == 3 ? argv[2] : "A";

    int ask_A = 0, ask_AAAA = 0;

    if (strcmp(query_type, "A") == 0) {
        ask_A = 1;
    } else if (strcmp(query_type, "AAAA") == 0) {
        ask_AAAA = 1;
    } else if (strcmp(query_type, "BOTH") == 0) {
        ask_A = 1;
        ask_AAAA = 1;
    } else {
        fprintf(stderr, "Unknown query type: %s. Use A, AAAA, or BOTH.\n", query_type);
        return EXIT_FAILURE;
    }

    srand(time(NULL));

    // Construire l'en-tête DNS
    uint16_t header[6] = {0};
    uint16_t id = rand() % 65535;
    uint16_t qr = 0;
    uint16_t opcode = 0;
    uint16_t aa = 0;
    uint16_t tc = 0;
    uint16_t rd = 1;
    uint16_t ra = 0;
    uint16_t z = 0;
    uint16_t rcode = 0;

    int qdcount = ask_A + ask_AAAA;
    header[0] = htons(id); // ID
    header[1] = htons((qr << 15) | (opcode << 11) | (aa << 10) | (tc << 9) | (rd << 8) | (ra << 7) | (z << 4) | rcode); // Flags
    header[2] = htons(qdcount); // QDCOUNT
    header[3] = htons(0); // ANCOUNT
    header[4] = htons(0); // NSCOUNT
    header[5] = htons(0); // ARCOUNT

    unsigned char *qname_A = NULL, *qname_AAAA = NULL;
    size_t qname_len_A = 0, qname_len_AAAA = 0;

    // Construire les questions DNS
    if (ask_A) {
        build_question(domain, 1, &qname_A, &qname_len_A); // QTYPE = 1 pour A
    }

    if (ask_AAAA) {
        build_question(domain, 28, &qname_AAAA, &qname_len_AAAA); // QTYPE = 28 pour AAAA
    }

    // Créer le socket UDP
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        free(qname_A);
        free(qname_AAAA);
        return EXIT_FAILURE;
    }

    struct sockaddr_in dest;
    
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(DNS_PORT);
    
    if (inet_pton(AF_INET, DNS_SERVER_IP, &dest.sin_addr) <= 0) {
        perror("inet_pton");
        free(qname_A);
        free(qname_AAAA);
        close(sockfd);
        return EXIT_FAILURE;
    }

    // Construire le message DNS
    size_t msg_len = sizeof(header) + qname_len_A + qname_len_AAAA;
    char *msg = malloc(msg_len);
    if (msg == NULL) {
        perror("malloc");
        free(qname_A);
        free(qname_AAAA);
        close(sockfd);
        return EXIT_FAILURE;
    }

    // Copie des différentes parties du message DNS dans le buffer
    unsigned char *p = (unsigned char *)msg;
    memcpy(p, header, sizeof(header)); // Copie de l'en-tête
    p += sizeof(header);

    if (ask_A) {
        memcpy(p, qname_A, qname_len_A); // Copie de la question de type A
        p += qname_len_A;
    }

    if (ask_AAAA) {
        memcpy(p, qname_AAAA, qname_len_AAAA); // Copie de la question de type AAAA
    }

    // Envoyer le message DNS
    if (sendto(sockfd, msg, msg_len, 0, (struct sockaddr *)&dest, sizeof(dest)) < 0) {
        perror("sendto");
        free(qname_A);
        free(qname_AAAA);
        free(msg);
        close(sockfd);
        return EXIT_FAILURE;
    }

    printf("DNS query sent to %s for domain %s\n", DNS_SERVER_IP, domain);

    // Recevoir la réponse
    char response[512];
    socklen_t len = sizeof(dest);
    int n = recvfrom(sockfd, response, sizeof(response), 0, (struct sockaddr *)&dest, &len);
    if (n < 0) {
        perror("recvfrom");
        free(qname_A);
        free(qname_AAAA);
        free(msg);
        close(sockfd);
        return EXIT_FAILURE;
    }

    printf("Received %d bytes from DNS server\n", n);

    // Analyser la réponse
    uint16_t *resp_header = (uint16_t *)response;
    uint16_t flags = ntohs(resp_header[1]);
    uint16_t qdcount_resp = ntohs(resp_header[2]);
    uint16_t ancount = ntohs(resp_header[3]);

    uint16_t rcode_resp = flags & 0x000F; // Récupérer le code de réponse

    if (rcode_resp != 0) {
        printf("Error: RCODE = %d\n", rcode_resp);
        switch (rcode_resp) {
            case 1: printf("Format error\n"); break;
            case 2: printf("Server failure\n"); break;
            case 3: printf("Name error\n"); break;
            case 4: printf("Not implemented\n"); break;
            case 5: printf("Refused\n"); break;
            default: printf("Unknown error\n"); break;
        }
        free(qname_A);
        free(qname_AAAA);
        free(msg);
        close(sockfd);
        return EXIT_FAILURE;
    }

    // Passer la section des questions
    unsigned char *reader = (unsigned char *)(response + sizeof(header));
    for (int i = 0; i < qdcount_resp; i++) {
        int offset;
        unsigned char *name = (unsigned char *)read_name(reader, (unsigned char *)response, &offset);
        reader += offset;
        free(name);
        reader += 4; // Sauter le champ QTYPE (2 octets) et QCLASS (2 octets)
    }

    // Lire les réponses
    for (int i = 0; i < ancount; i++) {
        int offset;
        unsigned char *name = (unsigned char *)read_name(reader, (unsigned char *)response, &offset);
        reader += offset;

        uint16_t type = ntohs(*(uint16_t *)reader);
        reader += 2;
        uint16_t class = ntohs(*(uint16_t *)reader);
        reader += 2;
        reader += 4; // Sauter le TTL
        uint16_t rdlength = ntohs(*(uint16_t *)reader);
        reader += 2;

        if ((type == 1 || type == 28) && class == 1) { // Type A (1) ou AAAA (28) et Classe IN
            char ip[INET6_ADDRSTRLEN];
            if (type == 1) {
                inet_ntop(AF_INET, reader, ip, INET_ADDRSTRLEN);
            } else {
                inet_ntop(AF_INET6, reader, ip, INET6_ADDRSTRLEN);
            }
            printf("Address: %s\n", ip);
        }
        reader += rdlength;
        free(name);
    }
    
    // Fermer les ressources
    free(qname_A);
    free(qname_AAAA);
    free(msg);
    close(sockfd);

    return EXIT_SUCCESS;
}
