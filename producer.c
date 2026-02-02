/* =========================
   producer.c
   Task 4.2: Producer
   -f filename (default stdin)
   -b buffer_size (default 4096)
   Reads data and writes to stdout
   + SIGINT graceful shutdown
   + SIGUSR1 print stats (bytes copied) to stderr
   ========================= */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
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
    const char *filename = NULL;
    size_t buffer_size = 4096;

    int opt;
    while ((opt = getopt(argc, argv, "f:b:")) != -1) {
        switch (opt) {
            case 'f':
                filename = optarg;
                break;
            case 'b': {
                long v = strtol(optarg, NULL, 10);
                if (v <= 0) {
                    fprintf(stderr, "Invalid buffer size.\nUsage: %s [-f file] [-b size]\n", argv[0]);
                    return 1;
                }
                buffer_size = (size_t)v;
                break;
            }
            default:
                fprintf(stderr, "Usage: %s [-f file] [-b size]\n", argv[0]);
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

    FILE *in = stdin;
    if (filename) {
        in = fopen(filename, "rb");
        if (!in) {
            perror("fopen");
            return 1;
        }
    }

    unsigned char *buf = (unsigned char *)malloc(buffer_size);
    if (!buf) {
        fprintf(stderr, "malloc failed\n");
        if (in != stdin) fclose(in);
        return 1;
    }

    unsigned long long total_bytes = 0;

    while (!shutdown_flag) {
        if (stats_flag) {
            fprintf(stderr, "[producer] bytes_sent=%llu\n", total_bytes);
            stats_flag = 0;
        }

        size_t n = fread(buf, 1, buffer_size, in);
        if (n > 0) {
            size_t off = 0;
            while (off < n) {
                ssize_t w = write(STDOUT_FILENO, buf + off, n - off);
                if (w < 0) {
                    if (errno == EINTR) continue;
                    perror("write");
                    free(buf);
                    if (in != stdin) fclose(in);
                    return 1;
                }
                off += (size_t)w;
                total_bytes += (unsigned long long)w;
                if (shutdown_flag) break;
            }
        }

        if (n < buffer_size) {
            if (feof(in)) break;
            if (ferror(in)) {
                perror("fread");
                free(buf);
                if (in != stdin) fclose(in);
                return 1;
            }
        }
    }

    if (stats_flag) {
        fprintf(stderr, "[producer] bytes_sent=%llu\n", total_bytes);
        stats_flag = 0;
    }
    fprintf(stderr, "[producer] exiting (bytes_sent=%llu)\n", total_bytes);

    free(buf);
    if (in != stdin) fclose(in);
    return 0;
}
