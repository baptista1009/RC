#ifndef ARGS_H
#define ARGS_H

void print_usage(char *program_name);
void process_arguments(int argc, char *argv[], char **peerport, char **ds_ip, char **ds_port);

#endif