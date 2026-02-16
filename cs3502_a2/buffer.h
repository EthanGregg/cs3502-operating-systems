// ============================================
// buffer.h - Shared definitions
// ============================================
// Ethan Gregg
// CS3502
// Assignment 2

#ifndef BUFFER_H
#define BUFFER_H

// Required includes for both producer and consumer
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <errno.h>

// Constants for shared memory and semaphores
#define BUFFER_SIZE 10
#define SHM_KEY 0x1234
#define SEM_MUTEX "/sem_mutex"
#define SEM_EMPTY "/sem_empty"
#define SEM_FULL  "/sem_full"

// Item stored in the bounded buffer
typedef struct {
    int value;        // the data value
    int producer_id;  // which producer created it
} item_t;

// Shared circular buffer stored in shared memory
typedef struct {
    item_t buffer[BUFFER_SIZE];
    int head;   // next write position (producer)
    int tail;   // next read position (consumer)
    int count;  // current number of items
} shared_buffer_t;

#endif
