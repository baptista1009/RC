#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define DEFAULT_DS_IP "193.136.138.142"  // IP do "tejo"
#define DEFAULT_DS_PORT "59000"        // Porta do "tejo"

void print_usage(char *program_name) {
    fprintf(stderr, "Uso: %s -m peerport [-n DSIP] [-p DSport]\n", program_name);
}

// Validar se o UID tem exatamente 6 dígitos
int validar_uid(char *uid) {
    if (strlen(uid) != 6) 
        return 0;
    for (int i = 0; i < 6; i++) {
        if (!isdigit(uid[i]))
            return 0;
    }
    return 1;
}

// Verificar se a password tem exatamente 8 caracteres alfanuméricos
int validar_password(char *password) {
    if (strlen(password) != 8) 
        return 0;
    for (int i = 0; i < 8; i++) {
        if (!isalnum(password[i]))  
            return 0;
    }
    return 1;
}












/*

int main(int argc, char *argv[]) {
    char *peerport = NULL;
    char *ds_ip = DEFAULT_DS_IP;
    char *ds_port = DEFAULT_DS_PORT;
    int opcao;


}

*/