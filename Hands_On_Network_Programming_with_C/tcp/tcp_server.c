
#include <bits/types/struct_timeval.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define READ_SIZE 4096

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s [port]\n", argv[0]);
        return EXIT_SUCCESS;
    }

    struct addrinfo hints;
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bind_addr;
    if (getaddrinfo(NULL, argv[1], &hints, &bind_addr)) {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", errno);
        return EXIT_FAILURE;
    }

    int sock_listen = socket(bind_addr->ai_family, bind_addr->ai_socktype, bind_addr->ai_protocol);
    if (sock_listen < 0) {
        fprintf(stderr, "socket() failed. (%d)\n", errno);
        return EXIT_FAILURE;
    }

    int no = 0;
    if (setsockopt(sock_listen, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&no, sizeof(no)) < 0)
        fprintf(stderr, "setsockopt(IPV6_V6ONLY) failed\n");

    int yes = 1;
    if (setsockopt(sock_listen, SOL_SOCKET, SO_REUSEADDR, (void *)&yes, sizeof(yes)) < 0)
        fprintf(stderr, "setsockopt(SO_REUSEADDR) failed\n");

    if (bind(sock_listen, bind_addr->ai_addr, bind_addr->ai_addrlen)) {
        fprintf(stderr, "bind() failed. (%d)\n", errno);
        return EXIT_FAILURE;
    }

    freeaddrinfo(bind_addr);
    if (listen(sock_listen, 20)) {
        fprintf(stderr, "listen() failed. (%d)\n", errno);
        return EXIT_FAILURE;
    }

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(sock_listen, &fds);
    int max_socket = sock_listen;

    printf("Listening..\n");

    for (;;) {
        fd_set reads = fds;
        if (select(max_socket + 1, &reads, NULL, NULL, NULL) < 0) {
            fprintf(stderr, "select() failed, (%d)\n", errno);
            return EXIT_FAILURE;
        }

        for (int i = 0; i <= max_socket; ++i) {
            if (FD_ISSET(i, &reads)) {
                if (i == sock_listen) {
                    struct sockaddr_storage client_addr;
                    socklen_t client_addrlen = sizeof(client_addr);
                    int socket_client = accept(sock_listen, (struct sockaddr *)&client_addr, &client_addrlen);
                    if (socket_client < 0) {
                        fprintf(stderr, "accept() failed, (%d)\n", errno);
                        return EXIT_FAILURE;
                    }

                    FD_SET(socket_client, &fds);
                    max_socket = socket_client > max_socket ? socket_client : max_socket;

                    char addr_buf[100];
                    getnameinfo((struct sockaddr *)&client_addr, client_addrlen,
                                    addr_buf, sizeof(addr_buf), NULL, 0, NI_NUMERICHOST);
                    printf("New connection from %s\n", addr_buf);
                }
                else {
                    char readb[READ_SIZE] = { 0 };
                    ssize_t bytes_received = recv(i, readb, READ_SIZE, 0);
                    if (bytes_received < 1) {
                        struct sockaddr_storage closed_addr;
                        socklen_t closed_addrlen = sizeof(closed_addr);

                        getsockname(i, (struct sockaddr *)&closed_addr, &closed_addrlen);
                        char addr_buff[100] = { 0 };
                        getnameinfo((const struct sockaddr *)&closed_addr, closed_addrlen,
                                        addr_buff, 100, NULL, 0, NI_NUMERICHOST);

                        printf("Closing peer (%s)\n", addr_buff);
                        FD_CLR(i, &fds);
                        close(i);
                        continue;
                    }
                    printf("Recv %ld bytes\n", bytes_received);
                    send(i, readb, bytes_received, 0);
                }
            }
        }
    }

    close(sock_listen);

    return EXIT_SUCCESS;
}
