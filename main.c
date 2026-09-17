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
int validate_uid(char *uid) {
    if (strlen(uid) != 6) 
        return 0;
    for (int i = 0; i < 6; i++) {
        if (!isdigit(uid[i]))
            return 0;
    }
    return 1;
}

// Verificar se a password tem exatamente 8 caracteres alfanuméricos
int validate_password(char *password) {
    if (strlen(password) != 8) 
        return 0;
    for (int i = 0; i < 8; i++) {
        if (!isalnum(password[i]))  
            return 0;
    }
    return 1;
}


void process_arguments(int argc, char *argv[], char **peerport, char **ds_ip, char **ds_port) {
    int opt;
    
    while ((opt = getopt(argc, argv, "m:n:p:")) != -1) {
        switch (opt) {
            case 'm':
                *peerport = optarg;
                break;
            case 'n':
                *ds_ip = optarg;
                break;
            case 'p':
                *ds_port = optarg;
                break;
            default:
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (*peerport == NULL) {
        fprintf(stderr, "Erro: A porta do peer é obrigatória.\n");
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }
}

int establish_connection(char *ds_ip, char *ds_port, struct addrinfo **res) {
    struct addrinfo hints;
    int fd, errcode;

    fd = socket(AF_INET, SOCK_DGRAM, 0);  //UDP socket
    if (fd == -1) {
        perror("socket");
        return -1;
    }


    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;            // IPv4
    hints.ai_socktype = SOCK_DGRAM;       // UDP

    errcode = getaddrinfo(ds_ip, ds_port, &hints, res);
    if (errcode != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(errcode));
        close(fd);
        exit(1);
    }
    return fd;
}


ssize_t send_request(int fd, struct addrinfo *res, char *msg, char *buffer, int buffer_size) {
    ssize_t n,

    n = sendto(fd, msg, strlen(msg), 0, res->ai_addr, res->ai_addrlen);

    if (n == -1) {
        perror("sendto");
        return -1;
    }

    n = recvfrom(fd, buffer, buffer_size - 1, 0, NULL, NULL);
    if (n == -1) {
        perror("recvfrom");
        return -1;  
    }

    buffer[n] = '\0';
    return n;
}

void RLI_response (char *buffer, int *logged_in){
    char status[16];

    if (sscanf(buffer, "RLI %15s", status) != 1) {
        printf("Invalid reply from DS.\n");
        return;
    }

    if (strcmp(status, "OK") == 0) {
        printf("Login successful.\n");
        session->logged_in = 1;
    } else if (strcmp(status, "NOK") == 0) {
        printf("Login failed: wrong password.\n");
    } else if (strcmp(status, "REG") == 0) {
        printf("New user registered and login successful.\n");
        session->logged_in = 1;
    } else if (strcmp(status, "ERR") == 0) {
        printf("Invalid login request.\n");
    } else {
        printf("Unknown reply from DS: %s\n", status);
    }

    if (session->logged_in) {
        strcpy(session->uid, uid);
        strcpy(session->password, password);
    }
}


void RLO_response (char *buffer, int *logged_in){
    char status[16];

    if (sscanf(buffer, "RLO %15s", status) != 1) {
        printf("Invalid reply from DS.\n");
        return;
    }

    if (strcmp(status, "OK") == 0) {
        printf("Logout successful.\n");
        session->logged_in = 0;
    } else if (strcmp(status, "NLG") == 0) {
        printf("User was not logged in.\n");
        session->logged_in = 0;
    } else if (strcmp(status, "UNR") == 0) {
        printf("User is not registered.\n");
    } else if (strcmp(status, "WRP") == 0) {
        printf("Wrong password.\n");
    } else if (strcmp(status, "ERR") == 0) {
        printf("Invalid logout request.\n");
    } else {
        printf("Unknown reply from DS: %s\n", status);
    }
}


void RUR_response (char *buffer, int *logged_in){
    char status[16];

    if (sscanf(buffer, "RUR %15s", status) != 1) {
        printf("Invalid reply from DS.\n");
        return;
    }

    if (strcmp(status, "OK") == 0) {
        printf("Unregister successful.\n");
        session->logged_in = 0;
    } else if (strcmp(status, "NOK") == 0) {
        printf("User was not logged in.\n");
    } else if (strcmp(status, "UNR") == 0) {
        printf("User is not registered.\n");
    } else if (strcmp(status, "WRP") == 0) {
        printf("Wrong password.\n");
    } else if (strcmp(status, "ERR") == 0) {
        printf("Invalid unregister request.\n");
    } else {
        printf("Unknown reply from DS: %s\n", status);
    }
}







/*

int main(int argc, char *argv[]) {
    char *peerport = NULL;
    char *ds_ip = DEFAULT_DS_IP;
    char *ds_port = DEFAULT_DS_PORT;
    int opcao;


}

*/