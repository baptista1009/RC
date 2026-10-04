#include "common.h"
#include "resources.h"
#include "net.h"
#include "validate.h"

static void RPB_response(char *buffer) {
    char status[16];
    if (sscanf(buffer, "RPB %15s", status) != 1) { printf("Invalid reply from DS.\n"); return; }
    if (strcmp(status, "OK") == 0)       printf("Successful publication.\n");
    else if (strcmp(status, "NLG") == 0) printf("User not logged in.\n");
    else if (strcmp(status, "UNR") == 0) printf("User not registered.\n");
    else if (strcmp(status, "WRP") == 0) printf("Incorrect password.\n");
    else if (strcmp(status, "NOK") == 0) printf("Unsuccessful publication.\n");
    else if (strcmp(status, "ERR") == 0) printf("Invalid publish request.\n");
    else printf("Unknown reply from DS: %s\n", status);
}

static void RRM_response(char *buffer) {
    char status[16];
    if (sscanf(buffer, "RRM %15s", status) != 1) { printf("Invalid reply from DS.\n"); return; }
    if (strcmp(status, "OK") == 0)       printf("Successful removal.\n");
    else if (strcmp(status, "NLG") == 0) printf("User not logged in.\n");
    else if (strcmp(status, "UNR") == 0) printf("User not registered.\n");
    else if (strcmp(status, "WRP") == 0) printf("Incorrect password.\n");
    else if (strcmp(status, "NOK") == 0) printf("Resource not found.\n");
    else if (strcmp(status, "ERR") == 0) printf("Invalid remove request.\n");
    else printf("Unknown reply from DS: %s\n", status);
}

void handle_publish(int fd, struct addrinfo *res, Session *session) {
    char *filename = strtok(NULL, " \t");
    char *label    = strtok(NULL, " \t");
    char *extra    = strtok(NULL, " \t");

    if (!filename || !label) { printf("Correct usage: publish filename label\n"); return; }
    if (extra)               { printf("Error: Too many arguments for publish command.\n"); return; }
    if (!session->logged_in) { printf("Error: you must be logged in to publish.\n"); return; }

    if (!validate_filename(filename)) {
        printf("Error: invalid filename (max 24 chars, name.xxx).\n");
        return;
    }
    if (!validate_label(label)) {
        printf("Error: label must have 1-20 chars (letters, digits, - or _).\n");
        return;
    }

    struct stat st;
    if (stat(filename, &st) == -1 || !S_ISREG(st.st_mode)) {
        printf("Error: file '%s' not found in the local directory.\n", filename);
        return;
    }
    if (st.st_size > MAX_FSIZE) {
        printf("Error: file is larger than 10 MB.\n");
        return;
    }

    char msg[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    snprintf(msg, sizeof(msg), "PUB %s %s %s %ld %s\n",
             session->uid, session->password, filename, (long)st.st_size, label);

    if (send_request(fd, res, msg, response, sizeof(response)) != -1)
        RPB_response(response);
}

void handle_remove(int fd, struct addrinfo *res, Session *session) {
    char *filename = strtok(NULL, " \t");
    char *extra    = strtok(NULL, " \t");

    if (!filename)           { printf("Correct usage: remove filename\n"); return; }
    if (extra)               { printf("Error: Too many arguments for remove command.\n"); return; }
    if (!session->logged_in) { printf("Error: you must be logged in to remove.\n"); return; }
    if (!validate_filename(filename)) {
        printf("Error: invalid filename (max 24 chars, name.xxx).\n");
        return;
    }

    char msg[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    snprintf(msg, sizeof(msg), "REM %s %s %s\n",
             session->uid, session->password, filename);

    if (send_request(fd, res, msg, response, sizeof(response)) != -1)
        RRM_response(response);
}