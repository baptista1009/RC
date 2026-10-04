#ifndef NET_H
#define NET_H

#include <netdb.h>
#include <sys/types.h>

/* Cria o socket UDP e resolve o endereco do DS. Devolve fd ou -1. */
int establish_connection(const char *ds_ip, const char *ds_port, struct addrinfo **res);

/* Envia msg ao DS e espera resposta (timeout 5s). Devolve n bytes ou -1. */
ssize_t send_request(int fd, struct addrinfo *res, const char *msg, char *buffer, int buffer_size);

#endif