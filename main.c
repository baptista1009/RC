#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ctype.h>
#include <signal.h>

#define DEFAULT_DS_IP "193.136.138.142"  // IP do "tejo"
#define DEFAULT_DS_PORT "59000"        // Porta do "tejo"
#define BUFFER_SIZE 128

typedef struct {
    char uid[7];        // 6 dígitos + '\0'
    char password[9];   // 8 caracteres + '\0'
    int logged_in;      // 1 se logado, 0 caso contrário
} Session;

void print_usage(char *program_name) {
    fprintf(stderr, "Uso: %s -m peerport [-n DSIP] [-p DSport]\n", program_name);
}

static int g_fd = -1;
static struct addrinfo *g_res = NULL;

void handle_sigint(int signal) {
    (void)signal;
    printf("\nCaught SIGINT. Shutting down cleanly...\n");
    if (g_res != NULL)
        freeaddrinfo(g_res);
    if (g_fd != -1)
        close(g_fd);
    exit(0);
}


int validate_uid(char *uid) {
    if (strlen(uid) != 6) 
        return 0;
    for (int i = 0; i < 6; i++) {
        if (!isdigit(uid[i]))
            return 0;
    }
    return 1;
}

int validate_password(char *password) {
    if (strlen(password) != 8) 
        return 0;
    for (int i = 0; i < 8; i++) {
        if (!isalnum(password[i]))  
            return 0;
    }
    return 1;
}

int validate_port(char *port) {
    if (port == NULL || strlen(port) == 0 || strlen(port) > 5) {
        return 0; // Porta inválida
    }
    for (size_t i = 0; i < strlen(port); i++) {
        if (!isdigit(port[i])) {
            return 0; // Porta inválida
        }
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
        fprintf(stderr, "Error: the peer port is required.\n");
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    if (!validate_port(*peerport)) {
        fprintf(stderr, "Error: invalid peer port.\n");
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
    ssize_t n;

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

void RLI_response (char *buffer, Session *session, char *uid, char *password) {
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

void RLO_response (char *buffer, Session *session){
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

void RUR_response (char *buffer, Session *session){
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

void handle_login(int fd, struct addrinfo *res, Session *session, char *peerport) {
    char *uid = strtok(NULL, " \t");
    char *password = strtok(NULL, " \t");
    char *extra = strtok(NULL, " \t");

    if (!uid || !password) {
        printf("Correct usage: login UID password\n");
        return;
    }
    if (!validate_password(password)) {
        printf("Error: Password must have exactly 8 alphanumeric characters.\n");
        return;
    }

    if (extra) {
        printf("Error: Too many arguments for login command.\n");
        return;
    }

    if (!validate_uid(uid)) {
        printf("Error: UID must have 6 digits.\n");
        return;
    }

    if (session->logged_in) {
        printf("Error: a user is already logged in. Please logout first.\n");
        return;
    }

    char msg[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    
    // Construir a mensagem LIN UID password peerTCPport
    snprintf(msg, sizeof(msg), "LIN %s %s %s\n", uid, password, peerport);

    if (send_request(fd, res, msg, response, sizeof(response)) != -1) {
        RLI_response(response, session, uid, password);
    }
}

void handle_logout(int fd, struct addrinfo *res, Session *session) {
    char *extra = strtok(NULL, " \t");
    if (extra) {
        printf("Error: Too many arguments for logout command.\n");
        return;
    }

    if (!session->logged_in) {
        printf("Error: no user is currently logged in.\n");
        return;
    }

    char msg[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    
    // Construir a mensagem LOU UID password
    snprintf(msg, sizeof(msg), "LOU %s %s\n", session->uid, session->password);

    if (send_request(fd, res, msg, response, sizeof(response)) != -1) {
        RLO_response(response, session);
    }
}

void handle_unregister(int fd, struct addrinfo *res, Session *session) {
    char *extra = strtok(NULL, " \t");
    if (extra) {
        printf("Error: Too many arguments for unregister command.\n");
        return;
    }

    if (!session->logged_in) {
        printf("Error: you must be logged in to run unregister.\n");
        return;
    }

    char msg[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    
    // Construir a mensagem UNR UID password
    snprintf(msg, sizeof(msg), "UNR %s %s\n", session->uid, session->password);

    if (send_request(fd, res, msg, response, sizeof(response)) != -1) {
        RUR_response(response, session);
    }
}

int handle_exit(Session *session) {
    char *extra = strtok(NULL, " \t");
    if (extra != NULL) {
        printf("Error: too many arguments. Correct usage: exit\n");
        return 0;
    }

    if (session->logged_in) {
        printf("Error: You must logout before exiting.\n");
        return 0; // Não sai do programa
    } else {
        printf("Shutting down the application...\n");
        return 1; // Sai do ciclo
    }
}


int main(int argc, char *argv[]) {
    char *peerport = NULL;
    char *ds_ip = DEFAULT_DS_IP;
    char *ds_port = DEFAULT_DS_PORT;
    
    process_arguments(argc, argv, &peerport, &ds_ip, &ds_port);

    struct addrinfo *res = NULL;
    int fd = establish_connection(ds_ip, ds_port, &res);
    if (fd == -1) {
        exit(EXIT_FAILURE);
    }

    g_fd = fd;
    g_res = res;
    signal(SIGINT, handle_sigint);

    Session session = {"", "", 0};
    char line[BUFFER_SIZE];

    printf("NetBox Client Started (Phase I) \n");
    printf("Available commands: login UID password, logout, unregister, exit\n");    

    while (fgets(line, sizeof(line), stdin) != NULL) {
        line[strcspn(line, "\n")] = 0; 

        char *command = strtok(line, " ");
        if (command == NULL) {
            continue; 
        }

        if (strcmp(command, "login") == 0) {
            handle_login(fd, res, &session, peerport);
        } else if (strcmp(command, "logout") == 0) {
            handle_logout(fd, res, &session);
        } else if (strcmp(command, "unregister") == 0) {
            handle_unregister(fd, res, &session);
        } else if (strcmp(command, "exit") == 0) {
            if (handle_exit(&session)) {
                break; 
            }
        } else {
            printf("Unknown command: %s\n", command);
        }
    }
    freeaddrinfo(res);
    close(fd);
    return 0;
}