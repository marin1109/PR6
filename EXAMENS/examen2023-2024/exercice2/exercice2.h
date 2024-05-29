#ifndef EXERCICE2_H
#define EXERCICE2_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

void prepa_recept(int sock){
    struct sockaddr_in6 adr = {AF_INET6, htons(1212), 0, in6addr_any, 0};

    bind(sock, (struct sockaddr*)&adr, sizeof(adr));
    struct ipr6_mreq group;
    inet_pton(AF_INET6, "ff12::ae2:b", &group.ipv6mr_multiaddr);
    setsockopt(sock, IPPROTO_IPV6, IPV6_JOIN_GROUP, &group, sizeof(group));
}

#endif
