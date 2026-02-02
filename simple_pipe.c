/* =========================
   simple_pipe.c
   Task 4.1: Parent -> Child via one pipe
   ========================= */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main(void) {
    int pipefd[2];
    pid_t pid;
    char buffer[100];
    const char *message = "Hello from parent!";

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return 1;
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Child: reads
        close(pipefd[1]); // close write end
        ssize_t n = read(pipefd[0], buffer, sizeof(buffer) - 1);
        if (n < 0) {
            perror("read");
            close(pipefd[0]);
            return 1;
        }
        buffer[n] = '\0';
        printf("Child received: %s\n", buffer);
        close(pipefd[0]);
        return 0;
    } else {
        // Parent: writes
        close(pipefd[0]); // close read end
        ssize_t n = write(pipefd[1], message, strlen(message));
        if (n < 0) {
            perror("write");
            close(pipefd[1]);
            return 1;
        }
        close(pipefd[1]);
        waitpid(pid, NULL, 0);
        return 0;
    }
}
