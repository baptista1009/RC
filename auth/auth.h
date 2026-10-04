#ifndef AUTH_H
#define AUTH_H

#include "common.h"

void handle_login(int fd, struct addrinfo *res, Session *session, char *peerport);
void handle_logout(int fd, struct addrinfo *res, Session *session);
void handle_unregister(int fd, struct addrinfo *res, Session *session);
int  handle_exit(Session *session);  /* 1 se pode sair */

#endif