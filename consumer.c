/* =========================
   consumer.c
   Task 4.2: Consumer
   -n max_lines (optional)
   -v verbose (echo lines)
   Reads stdin, counts lines/chars
   Prints stats to stderr
   + SIGINT graceful shutdown
   + SIGUSR1 print current stats to stderr
   ========================= */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <signal.h>

static volatile sig_atomic_t shutdown_flag = 0;
static volatile sig_atomic_t stats_flag = 0;

static void handle_sigint(int sig) {
    (void)sig;
    shutdown_flag = 1;
}

static void handle_sigusr1(int sig) {
    (void)sig;
    stats_flag = 1;
}

int main(int argc, char *argv[]) {
    long max_lines = -1;   // no limit
    int verbose = 0;

    int opt;
    while ((opt = getopt(argc, argv, "n:v")) != -1) {
        switch (opt) {
            case 'n': {
                char *end = NULL;
                long v = strtol(optarg, &end, 10);
                if (!end || *end != '\0' || v < 0) {
                    fprintf(stderr, "Invalid -n value.\nUsage: %s [-n max] [-v]\n", argv[0]);
                    return 1;
                }
                max_lines = v;
                break;
            }
            case 'v':
                verbose = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-n max] [-v]\n", argv[0]);
                return 1;
        }
    }

    struct sigaction sa_int;
    memset(&sa_int, 0, sizeof(sa_int));
    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sigaction(SIGINT, &sa_int, NULL);

    struct sigaction sa_usr;
    memset(&sa_usr, 0, sizeof(sa_usr));
    sa_usr.sa_handler = handle_sigusr1;
    sigemptyset(&sa_usr.sa_mask);
    sigaction(SIGUSR1, &sa_usr, NULL);

    char *line = NULL;
    size_t cap = 0;

    unsigned long long lines = 0;
    unsigned long long chars = 0;

    while (!shutdown_flag) {
        if (stats_flag) {
            fprintf(stderr, "[consumer] lines=%llu chars=%llu\n", lines, chars);
            stats_flag = 0;
        }

        ssize_t nread = getline(&line, &cap, stdin);
        if (nread < 0) break;

        lines++;
        chars += (unsigned long long)nread;

        if (verbose) {
            // echo line to stdout (keeps pipeline behavior)
            fputs(line, stdout);
            fflush(stdout);
        }

        if (max_lines >= 0 && lines >= (unsigned long long)max_lines) {
            break;
        }
    }

    if (stats_flag) {
        fprintf(stderr, "[consumer] lines=%llu chars=%llu\n", lines, chars);
        stats_flag = 0;
    }

    fprintf(stderr, "[consumer] done: lines=%llu chars=%llu\n", lines, chars);

    free(line);
    return 0;
}
