#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define SIZE_MESS 100

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
			printf("connexion impossible sur : %d %lu\n", p->ai_addrlen, sizeof(struct sockaddr_in));
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

int main(int argc, char *argv[])
{
	/* initialisation de OpenSSL */
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();

	/* créer le contexte OpenSSL en disant qu'on veut négocier
	   le meilleur algorithme de chiffrement partagé avec le
	   serveur à la connexion */
	SSL_CTX *ctx = SSL_CTX_new(TLS_client_method());
	if (!ctx)
	{
		fprintf(stderr, "SSL_CTX_new() failed.\n");
		return 1;
	}

	/* connexion au serveur */
	struct sockaddr server_addr;
	int sock, adrlen;

	switch (get_server_addr(argv[1], argv[2], &sock, &server_addr, &adrlen))
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

	/* crée un nouvel objet SSL, (hostname pour SNI),
	   and démarrage du TLS/SSL handshake */
	SSL *ssl = SSL_new(ctx);
	if (!ssl)
	{
		fprintf(stderr, "echec SSL_new()\n");
		ERR_print_errors_fp(stderr);
		return 1;
	}
	/*if (!SSL_set_tlsext_host_name(ssl, argv[1])) {
		fprintf(stderr, "echec SSL_set_tlsext_host_name()\n");
		ERR_print_errors_fp(stderr);
		return 1;
		}*/
	SSL_set_fd(ssl, sock);
	if (SSL_connect(ssl) == -1)
	{
		fprintf(stderr, "echec SSL_connect()\n");
		ERR_print_errors_fp(stderr);

		return 1;
	}

	/* affiche le cryptage choisi */
	printf("SSL/TLS using %s\n", SSL_get_cipher(ssl));

	//*** envoie d'un message ***
	char buf[SIZE_MESS];
	memset(buf, 0, SIZE_MESS);

	sprintf(buf, "Hello le monde");
	int ecrit = 0;
	while (ecrit < strlen(buf))
		ecrit += SSL_write(ssl, buf + ecrit, strlen(buf));

	while (1)
	{
		int lu = SSL_read(ssl, buf, sizeof(buf));
		if (lu == 0)
		{
			printf("\n Connexion interrompue\n");
			break;
		}
		buf[lu] = 0;
		printf("Recu (%d octets): %s\n", lu, buf);
	}

	/* libération des ressources */
	printf("\nFermeture socket...\n");
	SSL_shutdown(ssl);
	close(sock);
	SSL_free(ssl);

	/* libération du contexte SSL */
	SSL_CTX_free(ctx);

	printf("Terminé\n");
	return 0;
}
