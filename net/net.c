#include "common.h"
#include "net.h"

static void set_recv_timeout(int fd) {
    struct timeval tv = {5, 0};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

int establish_connection(const char *ds_ip, const char *ds_port, struct addrinfo **res) {
    struct addrinfo hints;
    int fd, errcode;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        perror("socket");
        return -1;
    }
    set_recv_timeout(fd);

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    errcode = getaddrinfo(ds_ip, ds_port, &hints, res);
    if (errcode != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(errcode));
        close(fd);
        return -1;
    }
    return fd;
}

ssize_t send_request(int fd, struct addrinfo *res, const char *msg, char *buffer, int buffer_size) {
    char junk[BUFFER_SIZE];
    while (recvfrom(fd, junk, sizeof(junk), MSG_DONTWAIT, NULL, NULL) > 0)
        ;  /* limpa respostas antigas pendentes */

    if (sendto(fd, msg, strlen(msg), 0, res->ai_addr, res->ai_addrlen) == -1) {
        perror("sendto");
        return -1;
    }

    ssize_t n = recvfrom(fd, buffer, buffer_size - 1, 0, NULL, NULL);
    if (n == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            printf("Timeout: no reply from DS.\n");
        else
            perror("recvfrom");
        return -1;
    }
    buffer[n] = '\0';
    return n;
}