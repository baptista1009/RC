#ifndef RESOURCES_H
#define RESOURCES_H

#include "common.h"

void handle_publish(int fd, struct addrinfo *res, Session *session);
void handle_remove(int fd, struct addrinfo *res, Session *session);

#endif