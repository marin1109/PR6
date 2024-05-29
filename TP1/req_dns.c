#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <string.h>

// Fonction pour encoder un domaine en format DNS
void encode_domain(const char *domain, uint8_t *buffer, size_t *len) {
    const char *pos = domain;
    const char *start = domain;
    uint8_t *ptr = buffer;

    while (*pos) {
        if (*pos == '.') {
            size_t label_len = pos - start;
            *ptr++ = label_len;
            memcpy(ptr, start, label_len);
            ptr += label_len;
            start = pos + 1;
        }
        pos++;
    }

    // Dernière étiquette
    size_t label_len = pos - start;
    *ptr++ = label_len;
    memcpy(ptr, start, label_len);
    ptr += label_len;

    *ptr++ = 0; // Caractère '\0' final

    *len = ptr - buffer;
}

int main(int argc, char *argv[]) {
    // Initialisation du générateur de nombres aléatoires
    srand(time(NULL));

    // Vérification du nombre d'arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <domain> <qtype>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Ouverture du fichier en écriture binaire
    FILE *file = fopen("requete", "wb");
    if (file == NULL) { 
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    uint16_t header[6];
    uint16_t QDCOUNT = 1;
    uint16_t ANCOUNT = 0;
    uint16_t NSCOUNT = 0;
    uint16_t ARCOUNT = 0;

    // Initialisation de l'en-tête DNS
    header[0] = htons(rand() % 65536); // Identifiant unique
    header[1] = htons(0x0100);         // Flags (requête standard)
    header[2] = htons(QDCOUNT);        // Nombre de questions
    header[3] = htons(ANCOUNT);        // Nombre de réponses
    header[4] = htons(NSCOUNT);        // Nombre d'enregistrements d'autorité
    header[5] = htons(ARCOUNT);        // Nombre d'enregistrements additionnels

    uint16_t QTYPE;
    if (strcmp(argv[2], "AAAA") == 0) {
        QTYPE = htons(28); // AAAA record
    } else {
        QTYPE = htons(1);  // A record par défaut
    }
    uint16_t QCLASS = htons(1); // IN (Internet)

    // Écriture de l'en-tête DNS dans le fichier
    if (fwrite(header, sizeof(uint16_t), 6, file) != 6) {
        perror("Error writing to file");
        fclose(file);
        return EXIT_FAILURE;
    }

    // Encodage du domaine
    const char *domain = argv[1];
    uint8_t domain_buffer[256];
    size_t domain_len;
    encode_domain(domain, domain_buffer, &domain_len);

    // Écriture du domaine encodé dans le fichier
    if (fwrite(domain_buffer, sizeof(uint8_t), domain_len, file) != domain_len) {
        perror("Error writing domain to file");
        fclose(file);
        return EXIT_FAILURE;
    }

    // Écriture du type de requête dans le fichier
    if (fwrite(&QTYPE, sizeof(uint16_t), 1, file) != 1) {
        perror("Error writing QTYPE to file");
        fclose(file);
        return EXIT_FAILURE;
    }

    // Écriture de la classe de requête dans le fichier
    if (fwrite(&QCLASS, sizeof(uint16_t), 1, file) != 1) {
        perror("Error writing QCLASS to file");
        fclose(file);
        return EXIT_FAILURE;
    }

    // Fermeture du fichier
    fclose(file);

    return 0;
}
