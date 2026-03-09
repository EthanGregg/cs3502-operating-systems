#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NUM_ACCOUNTS 2
#define INITIAL_BALANCE 1000.0
#define TRANSFER_AMOUNT 100.0

typedef struct {
    int account_id;
    double balance;
    int transaction_count;
    pthread_mutex_t lock;
} Account;

typedef struct {
    int from_id;
    int to_id;
    double amount;
} TransferArgs;

Account accounts[NUM_ACCOUNTS];
volatile int progress_made = 0;

void initialize_accounts(void) {
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i].account_id = i;
        accounts[i].balance = INITIAL_BALANCE;
        accounts[i].transaction_count = 0;
        pthread_mutex_init(&accounts[i].lock, NULL);
    }
}

void cleanup_mutexes(void) {
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        pthread_mutex_destroy(&accounts[i].lock);
    }
}

void transfer_deadlock(int from_id, int to_id, double amount) {
    pthread_mutex_lock(&accounts[from_id].lock);
    printf("Thread %lu: Locked account %d\n",
           (unsigned long)pthread_self(), from_id);
    fflush(stdout);

    usleep(100000);

    printf("Thread %lu: Waiting for account %d\n",
           (unsigned long)pthread_self(), to_id);
    fflush(stdout);

    pthread_mutex_lock(&accounts[to_id].lock);

    if (accounts[from_id].balance >= amount) {
        accounts[from_id].balance -= amount;
        accounts[to_id].balance += amount;
        accounts[from_id].transaction_count++;
        accounts[to_id].transaction_count++;
        progress_made = 1;
    }

    pthread_mutex_unlock(&accounts[to_id].lock);
    pthread_mutex_unlock(&accounts[from_id].lock);
}

void *transfer_thread(void *arg) {
    TransferArgs *t = (TransferArgs *)arg;
    transfer_deadlock(t->from_id, t->to_id, t->amount);
    return NULL;
}

int main(void) {
    printf("=== Phase 3: Deadlock Demonstration ===\n\n");

    initialize_accounts();

    printf("Initial balances:\n");
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("  Account %d: $%.2f\n", i, accounts[i].balance);
    }
    printf("\nCreating two transfers that will deadlock...\n\n");

    pthread_t t1, t2;
    TransferArgs a1 = {0, 1, TRANSFER_AMOUNT};
    TransferArgs a2 = {1, 0, TRANSFER_AMOUNT};

    if (pthread_create(&t1, NULL, transfer_thread, &a1) != 0) {
        perror("pthread_create");
        cleanup_mutexes();
        return 1;
    }

    if (pthread_create(&t2, NULL, transfer_thread, &a2) != 0) {
        perror("pthread_create");
        cleanup_mutexes();
        return 1;
    }

    time_t start = time(NULL);
    while (time(NULL) - start < 5) {
        if (progress_made) {
            break;
        }
        sleep(1);
    }

    if (!progress_made) {
        printf("\nSUSPECTED DEADLOCK DETECTED!\n");
        printf("No progress for 5 seconds.\n");
        printf("Thread 1 holds one account lock and waits for the other.\n");
        printf("Thread 2 holds the other account lock and waits back.\n");
        printf("This is circular wait.\n");
        fflush(stdout);
        return 0;
    }

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    cleanup_mutexes();
    return 0;
}
