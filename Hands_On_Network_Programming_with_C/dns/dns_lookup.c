#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRINGIFY(x) #x

void print_addrinfo(const struct addrinfo *addr) {
    char addr_buff[100];
    getnameinfo(addr->ai_addr, addr->ai_addrlen, addr_buff, sizeof(addr_buff),
                    0, 0, NI_NUMERICHOST);
    printf("addr   : %s\n", addr_buff);

    const char *str_family,
                *str_socktype,
                *str_protocol;

    switch (addr->ai_family) {
        case AF_INET:
            str_family = "IPv4"; break;
        case AF_INET6:
            str_family = "IPv6"; break;
    }

    switch (addr->ai_socktype) {
        case SOCK_RAW:
            str_socktype = STRINGIFY(SOCK_RAW); break;
        case SOCK_STREAM:
            str_socktype = STRINGIFY(SOCK_STREAM); break;
        case SOCK_DGRAM:
            str_socktype = STRINGIFY(SOCK_DGRAM); break;
    }

    switch (addr->ai_protocol) {
        case IPPROTO_TCP:
            str_protocol = "TCP"; break;
        case IPPROTO_UDP:
            str_protocol = "UDP"; break;
        case 0:
            str_protocol = "Null"; break;
    }


    printf("family : %s\n", str_family);
    printf("flags  : 0x%X\n", addr->ai_flags);
    printf("socktype : %s\n", str_socktype);
    printf("protocol : %s\n", str_protocol);
}

int main(int argc, char *argv[])
{

    if (argc < 2) {
        printf("Usage: %s hostname\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("Resolving hostname %s ...\n", argv[1]);

    struct addrinfo hints, *peer_addr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_ALL;

    if (getaddrinfo(argv[1], 0, &hints, &peer_addr)) {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", errno);
        return EXIT_FAILURE;
    }

    printf("Remote address:\n");
    struct addrinfo *addr = peer_addr;

    for (int i = 1; addr; addr = addr->ai_next) {
        printf("Entry #%d:\n", i++);
        print_addrinfo(addr);
        printf("\n");
    }
    freeaddrinfo(peer_addr);

    return EXIT_SUCCESS;
}
