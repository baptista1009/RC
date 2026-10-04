#include "common.h"
#include "auth.h"
#include "net.h"
#include "validate.h"

static void RLI_response(char *buffer, Session *session, char *uid, char *password) {
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

static void RLO_response(char *buffer, Session *session) {
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
        session->logged_in = 0;
    } else if (strcmp(status, "WRP") == 0) {
        printf("Wrong password.\n");
    } else if (strcmp(status, "ERR") == 0) {
        printf("Invalid logout request.\n");
    } else {
        printf("Unknown reply from DS: %s\n", status);
    }
}

static void RUR_response(char *buffer, Session *session) {
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
        session->logged_in = 0;
    } else if (strcmp(status, "UNR") == 0) {
        printf("User is not registered.\n");
        session->logged_in = 0;
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
    if (extra) {
        printf("Error: Too many arguments for login command.\n");
        return;
    }
    if (!validate_uid(uid)) {
        printf("Error: UID must have 6 digits.\n");
        return;
    }
    if (!validate_password(password)) {
        printf("Error: Password must have exactly 8 alphanumeric characters.\n");
        return;
    }
    if (session->logged_in) {
        printf("Error: a user is already logged in. Please logout first.\n");
        return;
    }

    char msg[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    snprintf(msg, sizeof(msg), "LIN %s %s %s\n", uid, password, peerport);

    if (send_request(fd, res, msg, response, sizeof(response)) != -1)
        RLI_response(response, session, uid, password);
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
    snprintf(msg, sizeof(msg), "LOU %s %s\n", session->uid, session->password);

    if (send_request(fd, res, msg, response, sizeof(response)) != -1)
        RLO_response(response, session);
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
    snprintf(msg, sizeof(msg), "UNR %s %s\n", session->uid, session->password);

    if (send_request(fd, res, msg, response, sizeof(response)) != -1)
        RUR_response(response, session);
}

int handle_exit(Session *session) {
    char *extra = strtok(NULL, " \t");
    if (extra != NULL) {
        printf("Error: too many arguments. Correct usage: exit\n");
        return 0;
    }
    if (session->logged_in) {
        printf("Error: You must logout before exiting.\n");
        return 0;
    }
    printf("Shutting down the application...\n");
    return 1;
}