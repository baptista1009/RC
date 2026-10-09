#include <signal.h>
#include "common.h"
#include "args.h"
#include "net.h"
#include "auth.h"
#include "resources.h"

static int g_fd = -1;
static struct addrinfo *g_res = NULL;

static void handle_sigint(int sig) {
    (void)sig;
    printf("\nCaught SIGINT. Shutting down cleanly...\n");
    if (g_res != NULL) freeaddrinfo(g_res);
    if (g_fd != -1) close(g_fd);
    exit(0);
}

int main(int argc, char *argv[]) {
    char *peerport = NULL;
    char *ds_ip = DEFAULT_DS_IP;
    char *ds_port = DEFAULT_DS_PORT;

    process_arguments(argc, argv, &peerport, &ds_ip, &ds_port);

    struct addrinfo *res = NULL;
    int fd = establish_connection(ds_ip, ds_port, &res);
    if (fd == -1) exit(EXIT_FAILURE);

    g_fd = fd;
    g_res = res;
    signal(SIGINT, handle_sigint);
    signal(SIGPIPE, SIG_IGN);   /* write() num socket TCP fechado nao pode matar o programa */

    Session session = {"", "", 0};
    char line[BUFFER_SIZE];

    printf("NetBox Client Started (Phase II)\n");
    printf("Commands: login UID password, logout, unregister, publish filename label,\n"
           "          remove filename, exit\n");

    while (fgets(line, sizeof(line), stdin) != NULL) {
        line[strcspn(line, "\n")] = 0;

        char *command = strtok(line, " \t");
        if (command == NULL) continue;

        if (strcmp(command, "login") == 0)            handle_login(fd, res, &session, peerport);
        else if (strcmp(command, "logout") == 0)      handle_logout(fd, res, &session);
        else if (strcmp(command, "unregister") == 0)  handle_unregister(fd, res, &session);
        else if (strcmp(command, "publish") == 0)     handle_publish(fd, res, &session);
        else if (strcmp(command, "remove") == 0)      handle_remove(fd, res, &session);
        else if (strcmp(command, "list") == 0)        handle_list(fd, res);
        else if (strcmp(command, "versions") == 0)    handle_versions(ds_ip, ds_port);
        else if (strcmp(command, "exit") == 0) {
            if (handle_exit(&session)) break;
        }
        else printf("Unknown command: %s\n", command);
    }
    freeaddrinfo(res);
    close(fd);
    return 0;
}