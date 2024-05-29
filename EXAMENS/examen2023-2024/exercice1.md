### On décrit un protocole de téléchargement en UDP d'un fichier, d'une application mono-agent B vers une application A :
#### 1. L'application A envoie sur le port 7777 un message composé uniquement d'un nom de fichier à l'application B.
#### 2. Si le fichier demandé n'est pas disponible(nous considèrons ici, que cela signifie qu'il n'existe pas de fichier du nom reçu dans le répertoire courant), l'application B répond par le message de 3 caractères NOK puis termine. A termine également dans ce cas. Si le fichier est disponible, l'application B commence par envoyer un message au format suivant :
```
'O' (8 octets) | 'K' (8 octets) | TAILLE_MAX (16 octets)
NB (32 octets)
```
où TAILLE_MAX et NB sont au format réseau. NB est le nombre messages envoyés par l'application B à l'application A et TAILLE_MAX est la taille maximale de chaque message envoyé. A récupère le nombre de messages envoyés et leur taille maximale.
#### 3. Ensuite l'application B envoie NB mesages correspondant aux données du fichier demandé. Chaque message est de taille TAILLE_MAX sauf le dernier qui peut être plus court. L'application A stocke les données reçues dans un fichier de même nom que celui du fichier demandé.
#### 4. Les deux applications terminent.

1. Quels problèmes peuvent survenir à l'étape 2? On ignorera ces problèmes dans la suite de l'exercice.
    - Sachant qu'on utilise une connexion de type UDP, nous ne pouvons pas être sur que le message envoyé par l'application A sera reçu par l'application B. Il est possible que le message soit perdu en cours de route. Pareille pour le message de retour de l'application B à l'application A.

2. Pourquoi le fichier téléchargé par l'application A peut être corrompu?
    - Le fichier téléchargé par l'application A peut être corrompu si un des messages envoyés par l'application B est perdu en cours de route. En effet, l'application A ne pourra pas reconstituer le fichier complet si un des messages est perdu.

3. Donner une valeur raisonnable pour TAILLE_MAX et justifier ce choix.
    - TAILLE_MAX peut être de 2¹⁶ - 1 octets, car la datagramme le spécifie ainsi. Mais on peut également choisir une valeur plus petite, pour éviter que des messages se perdent en cours de route.

### On suppose dans tout l'exercice que l'on dispose des fonctions suivantes :
    - int fic_existe(char *nomfic) qui prend un nom de fichier en paramètre et retourne 1 si un fichier de ce nom est présent dans le répertoire courant, 0 sinon.
    - int fic_taille(char *nomfic) qui prend une référence d'un fichier en paramètre et retourne la taille du fichier correspondant si celui-ci existe, -1 sinon.
    - int cree_fic(char *nomfic) qui crée ou écrase le fichier de référence nomfic. La fonction retourne 0 si tout s'est bien passé, 1 en cas de problème.
    - int lire_fic(char *nomfic, char *buf, int i, int tmax) qui lit sur le disque le fichier de référence nomfic à partir de l'octet i * tmax sur au plus tmax octets (ie. min(tmax, len-i)) où len est la longueur du fichier, et stocke les tmax octets lus dans buf qui doit être alloué au préalable. La fonction retourne le nombre d'octets lus, c'est-à-dire tmax sauf s'il y a moins d'octets à lire. En cas de problème, elle retourne -1.
    - int texte_append(char *nomfic, char *buf, int i, int tmax) qui ajoute au fichier de référence nomfic, à partir de la position i * tmax, la chaîne de caractères texte. La fonction retourne 0 si tout s'est bien passé, 1 en cas de problème.

### La communication entre A et B se fait sur IPv6. Les entités de A doivent communiquer avec une entité de B qui tourne sur une machine reliée à l'internet global et a les caractèristiques suivantes :
```bash
$ ip a
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default qlen 1000
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host noprefixroute
       valid_lft forever preferred_lft forever
2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc fq_codel state UP group default qlen 1000
    link/ether 7c:57:58:68:ae:a9 brd ff:ff:ff:ff:ff:ff
    altname eno1
    altname enp0s31f6
    inet 192.168.70.100/24 brd 192.168.70.255 scope global eth0
        valid_lft forever preferred_lft forever
    inet6 fdc7:9dd5:2c66:be86:7e57:58ff:fe68:aea9/64 scope global
        valid_lft forever preferred_lft forever
    inet6 fe80::7e57:58ff:fe68:aea9/64 scope link
        valid_lft forever preferred_lft forever
```
### On suppose, pour le moment, que les problèmes évoqués à la question 2 ne surviennent pas.

4. Écrire le code de l'application A correspondant aux étapes I et II, sachant que le nom du fichier à télécharger est passé en argument du programme. (~15 lignes)
```c
int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    int s = socket(AF_INET6, SOCK_DGRAM, 0);
    if (s == -1) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(7777);
    inet_pton(AF_INET6, "fdc7:9dd5:2c66:be86:7e57:58ff:fe68:aea9", &addr.sin6_addr);
    addr.sin6_scope_id = if_nametoindex("eth0");

    if (sendto(s, argv[1], strlen(argv[1]), 0, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("sendto");
        close(s);
        return 1;
    }

    char buf[1024];
    if (recvfrom(s, buf, sizeof(buf), 0, NULL, NULL) == -1) {
        perror("recvfrom");
        close(s);
        return 1;
    }
    if (strncmp(buf, "NOK", 3) == 0) {
        return (close(sock), 1);
    }
    u_int32_t nb; 
    u_int16_t taille_max;
    memcpy(&taille_max, &buf[16], sizeof(taille_max));
    taille_max = ntohs(taille_max);
    memcpy(&nb, &buf[32], sizeof(nb));
    nb = ntohl(nb);
    if(nb == 0) {
        close(s);
        return 0;
    }
}
```

5. Écrire le code de l'application B correspondant aux étapes I et II. (~20 lignes)
```c
int main() {
    int s = socket(AF_INET6, SOCK_DGRAM, 0);
    if (s == -1) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_port = htons(7777);
    addr.sin6_addr = in6addr_any;

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(s);
        return 1;
    }

    char buf[1024];
    struct sockaddr_in6 addr_cli;
    socklen_t len = sizeof(addr_cli);
    if (recvfrom(s, buf, sizeof(buf), 0, (struct sockaddr*)&addr_cli, &len) == -1) {
        perror("recvfrom");
        close(s);
        return 1;
    }
    if(!fic_existe(buf)) {
        if (sendto(s, "NOK", 3, 0, (struct sockaddr*)&addr_cli, len) == -1) {
            perror("sendto");
            close(s);
            return 1;
        }
        close(s);
        return 0;
    }
    uint16_t taille_max = htons(1024);
    uint32_t nb = htonl(fic_taille(buf) / 1024 + 1);
    char msg[48];
    strncpy(msg, "OK", 2);
    memcpy(msg + 16, &taille_max, sizeof(taille_max));
    memcpy(msg + 32, &nb, sizeof(nb));
    if (sendto(s, msg, sizeof(msg), 0, (struct sockaddr*)&addr_cli, len) == -1) {
        perror("sendto");
        return 1;
    }
}
```

6. Écrire le code de l'application A correspondant aux étapes III et IV. (~10 lignes)
```c
// code précédent de l'application A
if(cree_fic(argv[1]) != 1) {
    for(int i = 0; i < nb; i++) {
        char buf = calloc(taille_max+1, sizeof(char));
        if(recvfrom(s, buf, taille_max, 0, NULL, NULL) == -1) {
            perror("recvfrom");
            return 1;
        }
        if(texte_append(argv[1], buf, i, taille_max) == 1) {
            perror("texte_append");
            return 1;
        }
    }
}
close(s);
return 0;
```

7. Écrire le code de l'application B correspondant aux étapes III et IV. (~6 lignes)
```c
// code précédent de l'application B
nb = ntohl(nb);
for (int i = 0; i < nb; i++) {
    taille_max = ntohs(taille_max);
    char buf = calloc(taille_max, sizeof(char));
    if (lire_fic(argv[1], buf, i, taille_max) == -1) {
        perror("lire_fic");
        return 1;
    }
    if (sendto(s, buf, strlen(buf), 0, (struct sockaddr*)&addr_cli, len) == -1) {
        perror("sendto");
        return 1;
    }
    free(buf);
}
```

### On veut maintenant modifier le protocole afin qu'il soit plus fiable. Pour cela, on modifie l'étape III du protocole :
#### l'application B envoie les NB messages, chacun étant précédé de son numéro codé sur 4 octets et au format réseau. Les messages sont numérotés dans l'ordre et à partir de 0. Chaque message est donc de taille TAILLE_MAX + 4 saud éventuellement le dernier qui peut être plus court. 
#### L'application A stocke les données reçues dans un fichier de même nom que celui du fichier demandé. Chaque fois qu'elle constate qu'un message numéroté manque, elle fait une demande de réémission du message en envoyant un message à B composé uniquement du numéro du message manquant. 

8. Modifiez le code de B correspondant à l'étape III en accord avec le protocole modifié. B devra en parallèle :
    - lire les demandes de réémission d'un message provenant de A et réémettre le message demandé vers A.
    - envoyer les messages numéroté à A.

Il devra par ailleurs attendre au moins 5 secondes à chaque fois qu'il pensera avoir tout envoyé afin de ne pas manquer des demandes de réémissions de A. Enfin, B ne devra créer aucun nouveau processus système ou léger. (~26 lignes)

Vous pourrez supposer que vous disposez de la fonction voir mess_num(int nb, char *mess) qui ajoute au début de mess les 4 octets de l'entier nb mis au format réseau.

```c
// code précédent de l'application B
nb = ntohl(nb);
int timeout = 5000;
struct pollfd fds[1];
fds[0].fd = s;
fds[0].events = POLLIN;
for (int i = 0; i < nb; i++) {
    taille_max = ntohs(taille_max);
    char buf = malloc(taille_max);
    memset(buf, 0, taille_max);
    if (lire_fic(argv[1], buf, i, taille_max) == -1) {
        perror("lire_fic");
        return 1;
    }
    char mess = malloc(taille_max + 4);
    memset(mess, 0, taille_max + 4);
    mess_num(i, mess);
    memcpy(mess + 4, buf, taille_max);
    if (sendto(s, mess, taille_max + 4, 0, (struct sockaddr*)&addr_cli, len) == -1) {
        perror("sendto");
        return 1;
    }
    free(mess);

    int val = poll(fds, 1, &timeout);
    if (val == -1) {
        perror("poll");
        return 1;
    }
    if (fds[0].revents & POLLIN) {
        char buf[4];
        if (recvfrom(s, buf, sizeof(buf), 0, NULL, NULL) == -1) {
            perror("recvfrom");
            return 1;
        }
        int num;
        memcpy(&num, buf, sizeof(num));
        num = ntohl(num);
        if (lire_fic(argv[1], buf, num, taille_max) == -1) {
            perror("lire_fic");
            return 1;
        }
        mess = malloc(taille_max + 4);
        memset(mess, 0, taille_max + 4);
        mess_num(num, mess);
        if (sendto(s, buf, taille_max + 4, 0, (struct sockaddr*)&addr_cli, len) == -1) {
            perror("sendto");
            return 1;
        }
        free(mess);
    }
    free(buf);
}
```
