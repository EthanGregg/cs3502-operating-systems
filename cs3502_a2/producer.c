// ============================================
// producer.c - Producer process
// ============================================
// Ethan Gregg
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
    printf("\nProducer: Caught signal %d, cleaning up...\n", sig);
    cleanup();
    exit(0);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <producer_id> <num_items>\n", argv[0]);
        return 1;
    }

    int producer_id = atoi(argv[1]);
    int num_items   = atoi(argv[2]);
    if (producer_id <= 0 || num_items < 0) {
        fprintf(stderr, "Producer: invalid arguments. producer_id must be > 0 and num_items >= 0\n");
        return 1;
    }

    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);

    srand((unsigned)time(NULL) + (unsigned)producer_id);

    // Attach to shared memory (create if needed)
    int created = 0;
    shm_id = shmget(SHM_KEY, sizeof(shared_buffer_t), IPC_CREAT | IPC_EXCL | 0666);
    if (shm_id >= 0) {
        created = 1;
    } else {
        if (errno != EEXIST) {
            perror("Producer: shmget");
            return 1;
        }
        shm_id = shmget(SHM_KEY, sizeof(shared_buffer_t), 0666);
        if (shm_id < 0) {
            perror("Producer: shmget existing");
            return 1;
        }
    }

    buffer = (shared_buffer_t*)shmat(shm_id, NULL, 0);
    if (buffer == (void*)-1) {
        perror("Producer: shmat");
        buffer = NULL;
        return 1;
    }

    // Initialize buffer only if we created shared memory
    if (created) {
        memset(buffer, 0, sizeof(*buffer));
        buffer->head  = 0;
        buffer->tail  = 0;
        buffer->count = 0;
    }

    // Open/Create semaphores
    mutex = sem_open(SEM_MUTEX, O_CREAT, 0644, 1);
    if (mutex == SEM_FAILED) { perror("Producer: sem_open mutex"); cleanup(); return 1; }

    empty = sem_open(SEM_EMPTY, O_CREAT, 0644, BUFFER_SIZE);
    if (empty == SEM_FAILED) { perror("Producer: sem_open empty"); cleanup(); return 1; }

    full = sem_open(SEM_FULL, O_CREAT, 0644, 0);
    if (full == SEM_FAILED) { perror("Producer: sem_open full"); cleanup(); return 1; }

    printf("Producer %d: Starting to produce %d items\n", producer_id, num_items);

    for (int i = 0; i < num_items; i++) {
        item_t item;
        item.value       = producer_id * 1000 + i;
        item.producer_id = producer_id;

        // Wait for empty slot, then mutex (order matters)
        if (sem_wait(empty) == -1) { perror("Producer: sem_wait empty"); break; }
        if (sem_wait(mutex) == -1) { perror("Producer: sem_wait mutex"); sem_post(empty); break; }

        // Critical section: add item
        buffer->buffer[buffer->head] = item;
        buffer->head = (buffer->head + 1) % BUFFER_SIZE;
        buffer->count++;

        printf("Producer %d: Produced value %d\n", producer_id, item.value);

        // Exit critical section and signal full
        if (sem_post(mutex) == -1) { perror("Producer: sem_post mutex"); }
        if (sem_post(full)  == -1) { perror("Producer: sem_post full");  }

        usleep((useconds_t)(rand() % 100000));
    }

    printf("Producer %d: Finished producing %d items\n", producer_id, num_items);
    cleanup();
    return 0;
}
