#ifndef RESOURCES_H
#define RESOURCES_H

#include "common.h"

void handle_publish(int fd, struct addrinfo *res, Session *session);
void handle_remove(int fd, struct addrinfo *res, Session *session);
void handle_list(int fd, struct addrinfo *res);
void handle_versions(const char *ds_ip, const char *ds_port);

#endif