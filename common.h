#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netdb.h>

#define DEFAULT_DS_IP   "193.136.138.142"  /* IP do "tejo" */
#define DEFAULT_DS_PORT "59000"            /* Porta do "tejo" */
#define BUFFER_SIZE     128
#define MAX_FSIZE       10000000L

typedef struct {
    char uid[7];        /* 6 digitos + '\0' */
    char password[9];   /* 8 caracteres + '\0' */
    int  logged_in;     /* 1 se logado, 0 caso contrario */
} Session;

#endif