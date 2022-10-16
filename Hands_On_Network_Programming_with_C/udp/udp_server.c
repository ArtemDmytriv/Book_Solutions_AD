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

    int no = 0;
    if (setsockopt(listen_sock, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&no, sizeof(no)) < 0)
        fprintf(stderr, "setsockopt(IPV6_V6ONLY) failed\n");

    int yes = 1;
    if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (void *)&yes, sizeof(yes)) < 0)
        fprintf(stderr, "setsockopt(SO_REUSEADDR) failed\n");

    if (bind(listen_sock, serv_addr->ai_addr, serv_addr->ai_addrlen)) {
        fprintf(stderr, "bind() failed. (%d)\n", errno);
        return EXIT_FAILURE;
    }
    freeaddrinfo(serv_addr);

    printf("Listenning on port %s...\n", service);

    struct sockaddr_storage client_addr;
    socklen_t client_addrlen = sizeof(client_addr);
    char buff[MAXBUFLEN] = {};
    char ACK[] = "ACK";
    int ACKlen = strlen(ACK);

    for (;;) {
        // Recv msg
        int bytes_received = recvfrom(listen_sock, buff, MAXBUFLEN, 0,
                (struct sockaddr*)&client_addr, &client_addrlen);

        char client_host_str[100] = {};
        char client_serv_str[100] = {};
        getnameinfo((const struct sockaddr *)&client_addr, client_addrlen,
                        client_host_str, 100, client_serv_str, 100, NI_NUMERICHOST | NI_NUMERICSERV);

        printf("Received %d bytes from ([%s]:%s) :\n<S>%.*s<E>\n",
                    bytes_received, client_host_str, client_serv_str, bytes_received, buff);
        // Send ACK
        int bytes_sent = sendto(listen_sock, ACK, ACKlen,
                        0, (struct sockaddr *)&client_addr, client_addrlen);
    }
    return 0;
}
