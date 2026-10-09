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

/* RLS status [filename]* */
static void RLS_response(char *buffer) {
    const char *sep = " \t\r\n";
    char *save = NULL;
    char *tok = strtok_r(buffer, sep, &save);
    if (!tok || strcmp(tok, "RLS") != 0) { printf("Invalid reply from DS.\n"); return; }
    tok = strtok_r(NULL, sep, &save);
    if (!tok) { printf("Invalid reply from DS.\n"); return; }
 
    if (strcmp(tok, "NOK") == 0) { printf("No resources are currently known.\n"); return; }
    if (strcmp(tok, "ERR") == 0) { printf("Invalid list request.\n"); return; }
    if (strcmp(tok, "OK") != 0)  { printf("Unknown reply from DS: %s\n", tok); return; }
 
    printf("Available resources:\n");
    int i = 0;
    while ((tok = strtok_r(NULL, sep, &save)) != NULL)
        printf("  %2d. %s\n", ++i, tok);
    if (i == 0) printf("  (empty list)\n");
}

/* RVR status [UID Fsize label publication_time availability]* */
static void RVR_response(char *buffer, const char *filename) {
    const char *sep = " \t\r\n";
    char *save = NULL;
    char *tok = strtok_r(buffer, sep, &save);
    if (!tok || strcmp(tok, "RVR") != 0) { printf("Invalid reply from DS.\n"); return; }
    tok = strtok_r(NULL, sep, &save);
    if (!tok) { printf("Invalid reply from DS.\n"); return; }
 
    if (strcmp(tok, "NOK") == 0) { printf("No peer is sharing '%s'.\n", filename); return; }
    if (strcmp(tok, "ERR") == 0) { printf("Invalid versions request.\n"); return; }
    if (strcmp(tok, "OK") != 0)  { printf("Unknown reply from DS: %s\n", tok); return; }
 
    printf("Versions of %s:\n", filename);
    printf("  %-8s %-10s %-20s %-20s %s\n", "UID", "Size(B)", "Label", "Published", "Status");
 
    while ((tok = strtok_r(NULL, sep, &save)) != NULL) {
        char *uid   = tok;
        char *fsize = strtok_r(NULL, sep, &save);
        char *label = strtok_r(NULL, sep, &save);
        if (!fsize || !label) { printf("Invalid reply from DS.\n"); return; }
 
        char when[64] = "";
        char *avail = NULL;
        while ((tok = strtok_r(NULL, sep, &save)) != NULL) {
            if (strcmp(tok, "AVL") == 0 || strcmp(tok, "NAV") == 0) { avail = tok; break; }
            if (strlen(when) + strlen(tok) + 2 < sizeof(when)) {
                if (when[0]) strcat(when, " ");
                strcat(when, tok);
            }
        }
        if (!avail) { printf("Invalid reply from DS.\n"); return; }
        printf("  %-8s %-10s %-20s %-20s %s\n", uid, fsize, label, when,
               strcmp(avail, "AVL") == 0 ? "available" : "unavailable");
    }
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

void handle_list(int fd, struct addrinfo *res) {
    char *extra = strtok(NULL, " \t");
    if (extra) { printf("Error: Too many arguments for list command.\n"); return; }
 
    char response[BUFFER_SIZE];
    if (send_request(fd, res, "LST\n", response, sizeof(response)) != -1)
        RLS_response(response);
}
 
void handle_versions(const char *ds_ip, const char *ds_port) {
    char *filename = strtok(NULL, " \t");
    char *extra    = strtok(NULL, " \t");
 
    if (!filename) { printf("Correct usage: versions filename\n"); return; }
    if (extra)     { printf("Error: Too many arguments for versions command.\n"); return; }
    if (!validate_filename(filename)) {
        printf("Error: invalid filename (max 24 chars, name.xxx).\n");
        return;
    }
 
    char msg[BUFFER_SIZE];
    snprintf(msg, sizeof(msg), "VRS %s\n", filename);
 
    enum { VBUF = 65536 };                 /* a lista de peers pode ser grande */
    char *response = malloc(VBUF);
    if (!response) { perror("malloc"); return; }
    if (tcp_request(ds_ip, ds_port, msg, response, VBUF) > 0)
        RVR_response(response, filename);
    free(response);
}
 
