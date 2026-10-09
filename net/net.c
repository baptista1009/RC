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

/* Pedido TCP ao DS: liga, envia msg, le ate '\n' ou fim da ligacao.
   write()/read() podem transferir menos bytes, por isso ha ciclos. */
int tcp_request(const char *ip, const char *port, const char *msg, char *buffer, int buffer_size) {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
 
    int err = getaddrinfo(ip, port, &hints, &res);
    if (err != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
        return -1;
    }
    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd == -1) { perror("socket"); freeaddrinfo(res); return -1; }
 
    struct timeval tv = {5, 0};
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
 
    if (connect(fd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("connect");
        freeaddrinfo(res); close(fd);
        return -1;
    }
    freeaddrinfo(res);
 
    size_t len = strlen(msg), sent = 0;
    while (sent < len) {
        ssize_t n = write(fd, msg + sent, len - sent);
        if (n == -1 && errno == EINTR) continue;
        if (n <= 0) { perror("write"); close(fd); return -1; }
        sent += n;
    }
 
    int total = 0;
    while (total < buffer_size - 1) {
        ssize_t n = read(fd, buffer + total, buffer_size - 1 - total);
        if (n == -1 && errno == EINTR) continue;
        if (n == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                printf("Timeout: no reply from DS.\n");
            else
                perror("read");
            close(fd);
            return -1;
        }
        if (n == 0) break;                      /* DS fechou a ligacao */
        total += n;
        if (buffer[total - 1] == '\n') break;   /* mensagem completa */
    }
    buffer[total] = '\0';
    close(fd);
    return total;
}
