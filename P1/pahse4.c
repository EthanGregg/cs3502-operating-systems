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

void safe_transfer_ordered(int from_id, int to_id, double amount) {
    int first = (from_id < to_id) ? from_id : to_id;
    int second = (from_id < to_id) ? to_id : from_id;

    pthread_mutex_lock(&accounts[first].lock);
    printf("Thread %lu: Locked account %d first\n",
           (unsigned long)pthread_self(), first);
    fflush(stdout);

    usleep(100000);

    pthread_mutex_lock(&accounts[second].lock);
    printf("Thread %lu: Locked account %d second\n",
           (unsigned long)pthread_self(), second);
    fflush(stdout);

    if (accounts[from_id].balance >= amount) {
        accounts[from_id].balance -= amount;
        accounts[to_id].balance += amount;
        accounts[from_id].transaction_count++;
        accounts[to_id].transaction_count++;

        printf("Thread %lu: Transferred $%.2f from %d to %d\n",
               (unsigned long)pthread_self(), amount, from_id, to_id);
    } else {
        printf("Thread %lu: Insufficient funds in account %d\n",
               (unsigned long)pthread_self(), from_id);
    }

    pthread_mutex_unlock(&accounts[second].lock);
    pthread_mutex_unlock(&accounts[first].lock);
}

void *transfer_thread(void *arg) {
    TransferArgs *t = (TransferArgs *)arg;
    safe_transfer_ordered(t->from_id, t->to_id, t->amount);
    return NULL;
}

int main(void) {
    printf("=== Phase 4: Deadlock Resolution with Lock Ordering ===\n\n");

    initialize_accounts();

    printf("Initial balances:\n");
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("  Account %d: $%.2f\n", i, accounts[i].balance);
    }

    printf("\nRunning opposite-direction transfers safely...\n\n");

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

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("\n=== Final Results ===\n");
    double total = 0.0;
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("  Account %d: $%.2f (%d transactions)\n",
               i, accounts[i].balance, accounts[i].transaction_count);
        total += accounts[i].balance;
    }

    printf("\nTotal money in system: $%.2f\n", total);
    printf("No deadlock occurred because both threads locked accounts in the same order.\n");

    cleanup_mutexes();
    return 0;
}
