/* =========================
   bidirectional.c
   Task 4.3: Parent <-> Child with two pipes
   Parent sends message -> child responds -> parent prints response
   ========================= */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

static void die(const char *msg) {
    perror(msg);
    exit(1);
}

int main(void) {
    int pipe1[2]; // parent -> child
    int pipe2[2]; // child -> parent

    if (pipe(pipe1) == -1) die("pipe1");
    if (pipe(pipe2) == -1) die("pipe2");

    pid_t pid = fork();
    if (pid == -1) die("fork");

    if (pid == 0) {
        // Child
        close(pipe1[1]); // close write end of pipe1
        close(pipe2[0]); // close read end of pipe2

        char buf[256];
        ssize_t n = read(pipe1[0], buf, sizeof(buf) - 1);
        if (n < 0) die("child read");
        buf[n] = '\0';

        printf("Child got: %s\n", buf);

        const char *reply = "Message received. Hello back from child!";
        if (write(pipe2[1], reply, strlen(reply)) < 0) die("child write");

        close(pipe1[0]);
        close(pipe2[1]);
        return 0;
    } else {
        // Parent
        close(pipe1[0]); // close read end of pipe1
        close(pipe2[1]); // close write end of pipe2

        const char *msg = "Hello from parent (bidirectional)";
        if (write(pipe1[1], msg, strlen(msg)) < 0) die("parent write");
        close(pipe1[1]); // important: signal EOF to child

        char buf[256];
        ssize_t n = read(pipe2[0], buf, sizeof(buf) - 1);
        if (n < 0) die("parent read");
        buf[n] = '\0';

        printf("Parent got reply: %s\n", buf);

        close(pipe2[0]);
        waitpid(pid, NULL, 0);
        return 0;
    }
}
