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
    if (argc > 3 || argc < 2) {
        fprintf(stderr, "Wrong arguments\n");
        printf("Usage: %s {ip} [port]\n", argv[0]);
        return EXIT_FAILURE;
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;

    struct addrinfo *serv_addr;
    const char *service = (argc == 3)? argv[2] : "8080";
    if (getaddrinfo(argv[1], service, &hints, &serv_addr)) {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", errno);
        return EXIT_FAILURE;
    }

    printf("Socket creating...\n");
    int serv_sock = socket(serv_addr->ai_family, serv_addr->ai_socktype, serv_addr->ai_protocol);
    if (serv_sock < 0) {
        fprintf(stderr, "socket() failed. (%d)\n", errno);
        return EXIT_FAILURE;
    }

    char buff[MAXBUFLEN] = {};
    int k = 0;
    printf("Waiting User input:\n");

    while (fgets(buff + k, MAXBUFLEN, stdin)) {
        int bytes_read = strlen(buff + k);
        if (bytes_read == 1) {
            break;
        }
        k += bytes_read;
    }

    printf("Sending %d bytes...\n", k);
    int sent = 0;
    if ((sent = sendto(serv_sock, buff, k, 0, serv_addr->ai_addr, serv_addr->ai_addrlen)) != k) {
        fprintf(stderr, "sendto() failed, sent bytes = %d. (%d)\n", sent, errno);
        return EXIT_FAILURE;
    }

    freeaddrinfo(serv_addr);
    return 0;
}
