/* compilation : gcc serveur_ssl.c -o serv -lssl -lcrypto */

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define SIZE_MESS 100

int main(int argc, char *argv[])
{
	/* initialisation de OpenSSL */
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();

	/* créer le contexte OpenSSL en négociant le meilleur algorithme
	   de chiffrement partagé avec le serveur à la connexion */
	SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
	if (!ctx)
	{
		fprintf(stderr, "SSL_CTX_new() failed.\n");
		return 1;
	}

	/* l'objet SSL_CTX ctx utilisera notre certificat signé par nous-même
	   et notre clé */
	if (!SSL_CTX_use_certificate_file(ctx, "serv.crt", SSL_FILETYPE_PEM) ||
		!SSL_CTX_use_PrivateKey_file(ctx, "serv.key", SSL_FILETYPE_PEM))
	{
		fprintf(stderr, "echec SSL_CTX_use_certificate_file()\n");
		ERR_print_errors_fp(stderr);
		return 1;
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

	if (sockclient < 0)
	{
		perror("accept");
		exit(1);
	}

	/* création d'un nouvel objet SSL à partir du contexte SSL. */
	SSL *ssl = SSL_new(ctx);
	if (!ctx)
	{
		fprintf(stderr, "échec de SSL_new().\n");
		return 1;
	}

	/* l'objet SSL est lié à la socket client */
	SSL_set_fd(ssl, sockclient);
	/* SSL_accept() établit une connexion TLS/SSL */
	if (SSL_accept(ssl) <= 0)
	{
		fprintf(stderr, "échec de SSL_accept().\n");
		ERR_print_errors_fp(stderr);
		return 1;
	}
	printf("connexion SSL avec %s\n", SSL_get_cipher(ssl));

	char buf[SIZE_MESS];
	memset(buf, 0, SIZE_MESS);
	int lu = 0;
	while (lu < 14)
	{
		lu += SSL_read(ssl, buf + lu, sizeof(buf));
	}
	buf[lu] = 0;
	printf("Recu (%d octets): %s\n", lu, buf);

	int ecrit = 0;
	while (ecrit < strlen(buf))
		ecrit += SSL_write(ssl, buf + ecrit, strlen(buf));

	/* libération des ressources */
	SSL_shutdown(ssl);
	close(sockclient);
	SSL_free(ssl);

	/* libération du contexte SSL */
	SSL_CTX_free(ctx);

	return 0;
}
