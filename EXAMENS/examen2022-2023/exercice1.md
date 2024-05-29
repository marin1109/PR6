#### Une application 1 veut envoyer un message au format suivant à une application 2 :


1. Donnez les tailles minimale et maximale du champs TEXTE.
    - La taille minimale est de 0 octet.
    - La taille maximale est de 2³² - 1 octets.

2. Écrire les lignes de code de l’application 1 pour initier une connexion IPv4 ou IPv6 (les deux doivent être possible) à la machine hope sur le port 4444.
On ne demande pas d’écrire le code d’envoi du message.
    
```c
1   struct addrinfo hints, *p, *res;
2   int status;
3   int sock;
4
5   memset(&hints, 0, sizeof(hints));
6   hints.ai_family = AF_UNSPEC;
7   hints.ai_socktype = SOCK_STREAM;
8   if ((status = getaddrinfo("hope", "4444", &hints, &res)) != 0) {
9       fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
10      return 2;
11  }
12  p = res;
13  while (p != NULL) {
14      if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) != -1) {
15          if (connect(sock, p->ai_addr, p->ai_addrlen) != -1) {
16              break;
17          }
18          close(sock);
19      }
20      p = p->ai_next;
21  }
22  if (p == NULL) {
23      fprintf(stderr, "failed to connect\n");
24      return 2;
25  }
26  freeaddrinfo(res);
```

3. On suppose que ces deux applications ont établi une connexion TCP et que la socket de communication de l’application 2 se nomme sock2. Écrire les lignes de code de l’application 2 pour recevoir ce message et afficher son champs TEXTE.

```c
1   uint32_t len;
2   ssize_t n;
3   n = recv(sock2, &len, sizeof(len), 0);
4   if (n == -1 || n != sizeof(len)) {
5       perror("recv");
6       return 2;
7   }
8   len = ntohl(len);
9   char *buffer = malloc(len + 1);
10  if (buffer == NULL) {
11      perror("malloc");
12      return 2;
13  }
14  if (recv(sock2, buffer, len, 0) == -1) {
15      perror("recv");
16      free(buffer);
17      return 2;
18  }
19  buffer[len] = '\0';
20  printf("%s\n", buffer);
21  free(buffer);
```
