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
    if (argc != 3) {
        printf("Usage: %s [ip] [port]\n", argv[0]);
        return EXIT_SUCCESS;
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *peer_addr = NULL;
    if (getaddrinfo(argv[1], argv[2], &hints, &peer_addr)) {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", errno);
        return EXIT_FAILURE;
    }

    // Debug output
    char address_buffer[100] = { 0 };
    char service_buffer[100] = { 0 };
    getnameinfo(peer_addr->ai_addr, peer_addr->ai_addrlen,
                    address_buffer, sizeof(address_buffer),
                    service_buffer, sizeof(service_buffer), NI_NUMERICHOST);
    printf("%s %s\n", address_buffer, service_buffer);

    // Create socket

    int peer_sock = socket(peer_addr->ai_family, peer_addr->ai_socktype, peer_addr->ai_protocol);
    if (peer_sock < 0) {
        fprintf(stderr, "socket() failed. (%d)\n", errno);
        return EXIT_FAILURE;
    }

    if (connect(peer_sock, peer_addr->ai_addr, peer_addr->ai_addrlen)) {
        fprintf(stderr, "connect() failed. (%d)\n", errno);
        return EXIT_FAILURE;
    }

    // Use select for wait to Server or User terminal write
    fd_set ncfd;

    FD_ZERO(&ncfd);
    FD_SET(STDIN_FILENO, &ncfd);
    FD_SET(peer_sock, &ncfd);

    char rdbuf[READ_SIZE];
    for(;;) {
        fd_set rfds = ncfd;
        int retval = select(peer_sock + 1, &rfds, NULL, NULL, NULL);

        if (retval < 1) {
            fprintf(stderr, "select() failed. (%d)\n", errno);
            break;
        }
        else if (FD_ISSET(peer_sock, &rfds)) {
            ssize_t bytes_received = recv(peer_sock, rdbuf, READ_SIZE, 0);
            if (bytes_received < 1) {
                printf("Connection closed by peer\n");
                break;
            }
            printf("Received : %ld bytes\n%.*s", bytes_received, (int)bytes_received, rdbuf);
        }
        else if (FD_ISSET(STDIN_FILENO, &rfds)) {
            if (!fgets(rdbuf, READ_SIZE, stdin)) {
                break;
            }
            ssize_t bytes_sent = send(peer_sock, rdbuf, strlen(rdbuf), 0);
            printf("Send bytes : %ld\n", bytes_sent);
        }
    }

    struct timeval tv;
    tv.tv_usec = 250000;
    tv.tv_sec = 0;

    FD_CLR(STDIN_FILENO, &ncfd);
    int retval = select(peer_sock + 1, &ncfd, NULL, NULL, &tv);
    if (retval) {
        ssize_t bytes_received = recv(peer_sock, rdbuf, READ_SIZE, 0);
        printf("Received : %ld bytes\n%.*s", bytes_received, (int)bytes_received, rdbuf);
    }

    printf("\nClosing connection\n");
    close(peer_sock);
    freeaddrinfo(peer_addr);

    return EXIT_SUCCESS;
}
