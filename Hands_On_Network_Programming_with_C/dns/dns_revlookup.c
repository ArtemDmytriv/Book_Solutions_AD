
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

int main(int argc, char *argv[])
{

    if (argc < 2) {
        printf("Usage: %s {ip4|ip6}\n", argv[0]);
        return EXIT_FAILURE;
    }

    struct addrinfo *addr;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_NUMERICHOST;
    hints.ai_family = AF_UNSPEC;

    if (getaddrinfo(argv[1], NULL, &hints, &addr)) {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", errno);
        return EXIT_FAILURE;
    }

    for (struct addrinfo *current = addr; current ; current = current->ai_next) {
        char name[256] = {};
        int ret = getnameinfo(addr->ai_addr, addr->ai_addrlen, name, sizeof(name), NULL, 0, NI_NUMERICSERV);
        if (!ret) {
            printf("IP: %s\n", argv[1]);
            printf("Result: %s\n", name);
            break;
        }
    }
    freeaddrinfo(addr);
    return EXIT_SUCCESS;
}
