#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXBUFLEN 4096

int main(int argc, char *argv[])
{
    if (argc > 2) {
        fprintf(stderr, "Wrong arguments\n");
        printf("Usage: %s [port]\n", argv[0]);
        return EXIT_FAILURE;
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *serv_addr;
    const char *service = (argc == 2)? argv[1] : "8080";

    if (getaddrinfo(0, service, &hints, &serv_addr)) {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", errno);
        return EXIT_FAILURE;
    }

    printf("Socket creating...\n");
    int listen_sock = socket(serv_addr->ai_family, serv_addr->ai_socktype, serv_addr->ai_protocol);
    if (listen_sock < 0) {
        fprintf(stderr, "socket() failed. (%d)\n", errno);
        return EXIT_FAILURE;
    }

    if (bind(listen_sock, serv_addr->ai_addr, serv_addr->ai_addrlen)) {
        fprintf(stderr, "bind() failed. (%d)\n", errno);
        return EXIT_FAILURE;
    }
    freeaddrinfo(serv_addr);

    printf("Listenning on port %s...\n", service);

    int yes = 1;
    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (void *)&yes, sizeof(yes)) < 0)
        fprintf(stderr, "setsockopt(SO_REUSEADDR) failed\n");


    struct sockaddr_storage client_addr;
    socklen_t client_addrlen = sizeof(client_addr);
    char buff[MAXBUFLEN] = {};

    int bytes_received = recvfrom(listen_sock, buff, MAXBUFLEN, 0,
            (struct sockaddr*)&client_addr, &client_addrlen);

    printf("Received %d bytes from ... :\n%.*s\n", bytes_received, bytes_received, buff);
    return 0;
}
