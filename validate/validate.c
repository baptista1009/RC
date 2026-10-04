#include "common.h"
#include "validate.h"

int validate_uid(const char *uid) {
    if (strlen(uid) != 6) return 0;
    for (int i = 0; i < 6; i++)
        if (!isdigit((unsigned char)uid[i])) return 0;
    return 1;
}

int validate_password(const char *password) {
    if (strlen(password) != 8) return 0;
    for (int i = 0; i < 8; i++)
        if (!isalnum((unsigned char)password[i])) return 0;
    return 1;
}

int validate_port(const char *port) {
    if (port == NULL || strlen(port) == 0 || strlen(port) > 5) return 0;
    for (size_t i = 0; port[i]; i++)
        if (!isdigit((unsigned char)port[i])) return 0;
    int p = atoi(port);
    return p >= 1 && p <= 65535;
}

int validate_filename(const char *filename) {
    if (filename == NULL || strlen(filename) > 24) return 0;

    const char *dot = strrchr(filename, '.');
    if (dot == NULL || dot == filename) return 0;

    if (strlen(dot + 1) != 3) return 0;
    for (int i = 0; i < 3; i++)
        if (!isalnum((unsigned char)dot[1 + i])) return 0;

    size_t base_len = dot - filename;
    for (size_t i = 0; i < base_len; i++) {
        char c = filename[i];
        if (!isalnum((unsigned char)c) && c != '-' && c != '_') return 0;
    }
    return 1;
}

int validate_label(const char *label) {
    size_t n = label ? strlen(label) : 0;
    if (n < 1 || n > 20) return 0;
    for (size_t i = 0; i < n; i++) {
        char c = label[i];
        if (!isalnum((unsigned char)c) && c != '-' && c != '_') return 0;
    }
    return 1;
}