#include "common.h"
#include "args.h"
#include "validate.h"

void print_usage(char *program_name) {
    fprintf(stderr, "Uso: %s -m peerport [-n DSIP] [-p DSport]\n", program_name);
}

void process_arguments(int argc, char *argv[], char **peerport, char **ds_ip, char **ds_port) {
    int opt;

    while ((opt = getopt(argc, argv, "m:n:p:")) != -1) {
        switch (opt) {
            case 'm': *peerport = optarg; break;
            case 'n': *ds_ip = optarg; break;
            case 'p': *ds_port = optarg; break;
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
    if (!validate_port(*ds_port)) {
        fprintf(stderr, "Error: invalid DS port.\n");
        exit(EXIT_FAILURE);
    }
}