// ============================================
// consumer.c - Consumer process
// ============================================
//Ethan Gregg
// CS3502
// Assignment 2

#include "buffer.h"

// Global variables for cleanup
shared_buffer_t* buffer = NULL;
sem_t* mutex = SEM_FAILED;
sem_t* empty = SEM_FAILED;
sem_t* full  = SEM_FAILED;
int shm_id = -1;

static void cleanup() {
    if (buffer != NULL && buffer != (void*)-1) {
        shmdt(buffer);
        buffer = NULL;
    }
    if (mutex != SEM_FAILED) { sem_close(mutex); mutex = SEM_FAILED; }
    if (empty != SEM_FAILED) { sem_close(empty); empty = SEM_FAILED; }
    if (full  != SEM_FAILED) { sem_close(full);  full  = SEM_FAILED; }
}

static void signal_handler(int sig) {
    printf("\nConsumer: Caught signal %d, cleaning up...\n", sig);
    cleanup();
    exit(0);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <consumer_id> <num_items>\n", argv[0]);
        return 1;
    }

    int consumer_id = atoi(argv[1]);
    int num_items   = atoi(argv[2]);
    if (consumer_id <= 0 || num_items < 0) {
        fprintf(stderr, "Consumer: invalid arguments. consumer_id must be > 0 and num_items >= 0\n");
        return 1;
    }

    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);

    srand((unsigned)time(NULL) + (unsigned)consumer_id * 100U);

    // Attach to existing shared memory
    shm_id = shmget(SHM_KEY, sizeof(shared_buffer_t), 0666);
    if (shm_id < 0) {
        perror("Consumer: shmget (did you start a producer first?)");
        return 1;
    }

    buffer = (shared_buffer_t*)shmat(shm_id, NULL, 0);
    if (buffer == (void*)-1) {
        perror("Consumer: shmat");
        buffer = NULL;
        return 1;
    }

    // Open semaphores (producer creates them)
    mutex = sem_open(SEM_MUTEX, 0);
    if (mutex == SEM_FAILED) { perror("Consumer: sem_open mutex"); cleanup(); return 1; }

    empty = sem_open(SEM_EMPTY, 0);
    if (empty == SEM_FAILED) { perror("Consumer: sem_open empty"); cleanup(); return 1; }

    full = sem_open(SEM_FULL, 0);
    if (full == SEM_FAILED)  { perror("Consumer: sem_open full");  cleanup(); return 1; }

    printf("Consumer %d: Starting to consume %d items\n", consumer_id, num_items);

    for (int i = 0; i < num_items; i++) {
        // Wait for item, then mutex (order matters)
        if (sem_wait(full) == -1) { perror("Consumer: sem_wait full"); break; }
        if (sem_wait(mutex) == -1) { perror("Consumer: sem_wait mutex"); sem_post(full); break; }

        // Critical section: remove item
        item_t item = buffer->buffer[buffer->tail];
        buffer->tail = (buffer->tail + 1) % BUFFER_SIZE;
        buffer->count--;

        printf("Consumer %d: Consumed value %d from Producer %d\n",
               consumer_id, item.value, item.producer_id);

        // Exit critical section and signal empty
        if (sem_post(mutex) == -1) { perror("Consumer: sem_post mutex"); }
        if (sem_post(empty) == -1) { perror("Consumer: sem_post empty"); }

        usleep((useconds_t)(rand() % 100000));
    }

    printf("Consumer %d: Finished consuming %d items\n", consumer_id, num_items);
    cleanup();
    return 0;
}
