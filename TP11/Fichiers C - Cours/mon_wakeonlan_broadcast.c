#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>

uint8_t mystrtol_16(char *snb)
{
	uint8_t hex;
	for (int i = 0; i < 2; i++)
	{
		uint8_t c = *(snb + i);
		if ('0' <= c && c <= '9')
			c -= '0';
		else if (c != ':' && 'A' < c && c < 'Z')
			c -= 'A' - 10;
		else if (c != ':' && 'a' < c && c < 'z')
			c -= 'a' - 10;
		if (i == 0)
			hex = c << 4;
		else
			hex += c;
	}

	return hex;
}

int main(int argc, char *argv[])
{
	int sock = socket(PF_INET, SOCK_DGRAM, 0);

	int ok = 1;
	int r = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &ok, sizeof(ok));
	if (r == -1)
	{
		perror("setsockopt SO_BROADCAST");
		exit(1);
	}

	struct sockaddr_in adrdiff;
	memset(&adrdiff, 0, sizeof(adrdiff));
	adrdiff.sin_family = AF_INET;
	adrdiff.sin_port = htons(9);
	r = inet_pton(AF_INET, "255.255.255.255", &adrdiff.sin_addr);
	if (r <= 0)
	{
		perror("pb adresse");
		exit(1);
	}

	uint8_t paquet[102];

	/*** 6 premiers octets à ff ff ff ff ff ff ***/

	/*** methode 1 ***/
	/*** for(int i=0; i<6; i++) ***/
	/***   paquet[i] = 0xff; ***/

	/*** methode 2 ***/
	uint64_t nb = 0xffffffffffff;
	memcpy(paquet, &nb, 6);

	/*** 6 * 16 octets suivants contiennent 16 fois l'adresse mac ***/
	if (argc == 1)
	{
		// adresse MAC de hey-joe : 7C:57:58:6D:FF:F6
		for (int i = 1; i <= 16; i++)
		{
			paquet[i * 6] = 0x7C;
			paquet[i * 6 + 1] = 0x57;
			paquet[i * 6 + 2] = 0x58;
			paquet[i * 6 + 3] = 0x6D;
			paquet[i * 6 + 4] = 0xFF;
			paquet[i * 6 + 5] = 0xF6;
		}
	}
	else
	{
		char *mac = argv[1];
		/*** 1er stockage de l'adresse mac ***/
		for (int j = 0; j < 6; j++)
		{
			/*** methode 1 ***/
			char *suiv;
			nb = strtol(mac, &suiv, 16);
			memcpy(paquet + 6 + j, &nb, 1);
			if (*suiv != 0)
				mac = suiv + 1;
			else
				break;

			/*** methode 2 : en utilisant notre fonction str vers hexa ***/
			/*** uint8_t n = mystrtol_16(mac); ***/
			/*** memcpy(paquet+6+j, &n, 1); ***/
			/* mac = memchr(mac, ':', strlen(mac)); */
			/* if(mac != NULL) */
			/* 	mac += 1; */
			/* else */
			/* 	break; */
		}

		/*** 15 stockages suivants de l'adresse mac ***/
		for (int i = 2; i <= 16; i++)
		{
			memcpy(paquet + 6 * i, paquet + 6, 6);
		}
	}

	write(1, paquet, 102);

	r = sendto(sock, paquet, sizeof(paquet), 0, (struct sockaddr *)&adrdiff, (socklen_t)sizeof(struct sockaddr_in));
	if (r < 0)
	{
		perror("sendto");
		exit(1);
	}

	return 0;
}
