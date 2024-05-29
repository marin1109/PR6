// Q1

int main()
{
	struct addrinfo *first_info;
	struct addrinfo hints;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	int r = getaddrinfo("machine1.progreseaux.fr", "5678", &hints, &first_info);
	if (r == 0)
	{
		struct addrinfo *info = first_info;
		int found = 0;
		struct sockaddr_in *addressin;
		while (info != NULL && found == 0)
		{
			addressin = (struct sockaddr_in *)info->ai_addr;
			found = 1;
			info = info->ai_next;
		}
		if (found == 1)
		{

			int descr = socket(PF_INET, SOCK_STREAM, 0);
			int r2 = connect(descr, (struct sockaddr *)adressin,
							 sizeof(struct sockaddr_in));
			if (r2 != -1)
			{
				char *mess = "M Hello World!\n";
				send(descr, mess, strlen(mess), 0);
				close(descr);
			}
		}
	}
	return 0;
}

// Q2.

int requestLeaf()
{
	struct addrinfo *first_info;
	struct addrinfo hints;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;

	int r = getaddrinfo("machine1.progreseaux.fr", "5678", &hints, &first_info);
	if (r == 0)
	{
		struct addrinfo *info = first_info;
		int found = 0;
		struct sockaddr *saddr;
		struct sockaddr_in *addressin;
		while (info != NULL && found == 0)
		{
			addressin = (struct sockaddr_in *)info->ai_addr;
			found = 1;
			info = info->ai_next;
		}
		if (found == 1)
		{

			int descr = socket(PF_INET, SOCK_STREAM, 0);
			int r2 = connect(descr, (struct sockaddr *)adressin,
							 sizeof(struct sockaddr_in));
			if (r2 != -1)
			{
				char *mess = "LEAF\n";
				send(descr, mess, strlen(mess), 0);
				close(descr);
			}
		}
	}
	return 0;
}

// Q3.

char *getLeaf()
{
	int sock = socket(PF_INET, SOCK_DGRAM, 0);
	int ok = 1;
	int r = setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &ok, sizeof(ok));
	struct sockaddr_in address_sock;
	address_sock.sin_family = AF_INET;
	address_sock.sin_port = htons(4242);
	address_sock.sin_addr.s_addr = htonl(INADDR_ANY);
	r = bind(sock, (struct sockaddr *)&address_sock, sizeof(struct sockaddr_in));
	struct ip_mreq mreq;
	mreq.imr_multiaddr.s_addr = inet_addr("233.222.222.1");
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	r = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
	char tampon[100];
	int rec = recv(sock, tampon, 100, 0);
	tampon[rec] = '\0';
	return rec + 2;
}
return 0;
}

// Q4.

int nextLeaf(char *ip)
{
	struct sockaddr_in adress_sock;
	adress_sock.sin_family = AF_INET;
	adress_sock.sin_port = htons(5678);
	inet_aton(ip, &adress_sock.sin_addr);
	int descr = socket(PF_INET, SOCK_STREAM, 0);
	int r = connect(descr, (struct sockaddr *)&adress_sock,
					sizeof(struct sockaddr_in));
	if (r != -1)
	{
		char *mess = "CONNECT!\n";
		send(descr, mess, strlen(mess), 0);
		char buff[100];
		int size_rec = recv(descr, buff, 99 * sizeof(char), 0);
		buff[size_rec] = '\0';
		if (strcmp(rec, "OKSON\n") == 0)
		{
			close(descr);
			return 0;
		}
		else
		{
			close(descr);
			return -1;
		}
		close(descr);
	}
	return -1;
}

// Q5.

nt main()
{
	int found = 0;
	while (found == 0)
	{
		requestLeaf();
		char *ip = getLeaf();
		if (nextLeaf(ip) == 0)
		{
			found = 1;
		}
	}
	printf("I am leaf\n");
}
