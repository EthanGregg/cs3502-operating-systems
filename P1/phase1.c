#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define NUM_ACCOUNTS 2
#define NUM_THREADS 4
#define TRANSACTIONS_PER_THREAD 10
#define INITIAL_BALANCE 1000.0

typedef struct {
    int account_id;
    double balance;
    int transaction_count;
} Account;

Account accounts[NUM_ACCOUNTS];

void deposit_unsafe(int account_id, double amount) {
    double current_balance = accounts[account_id].balance;
    usleep(1);
    double new_balance = current_balance + amount;
    accounts[account_id].balance = new_balance;
    accounts[account_id].transaction_count++;
}

void withdrawal_unsafe(int account_id, double amount) {
    double current_balance = accounts[account_id].balance;
    usleep(1);
    double new_balance = current_balance - amount;
    accounts[account_id].balance = new_balance;
    accounts[account_id].transaction_count++;
}

void *teller_thread(void *arg) {
    int teller_id = *(int *)arg;
    unsigned int seed = (unsigned int)(time(NULL) ^ (unsigned long)pthread_self());

    for (int i = 0; i < TRANSACTIONS_PER_THREAD; i++) {
        int account_idx = rand_r(&seed) % NUM_ACCOUNTS;
        double amount = (double)((rand_r(&seed) % 100) + 1);
        int operation = rand_r(&seed) % 2;

        if (operation == 1) {
            deposit_unsafe(account_idx, amount);
            printf("Teller %d: Deposited $%.2f to Account %d\n",
                   teller_id, amount, account_idx);
        } else {
            withdrawal_unsafe(account_idx, amount);
            printf("Teller %d: Withdrew $%.2f from Account %d\n",
                   teller_id, amount, account_idx);
        }
    }

    return NULL;
}

int main(void) {
    printf("=== Phase 1: Race Conditions Demo ===\n\n");

    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i].account_id = i;
        accounts[i].balance = INITIAL_BALANCE;
        accounts[i].transaction_count = 0;
    }

    printf("Initial State:\n");
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("  Account %d: $%.2f\n", i, accounts[i].balance);
    }

    double initial_total = NUM_ACCOUNTS * INITIAL_BALANCE;
    printf("\nInitial total: $%.2f\n\n", initial_total);

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, teller_thread, &thread_ids[i]) != 0) {
            perror("pthread_create");
            return 1;
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
            return 1;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed =
        (end.tv_sec - start.tv_sec) +
        (end.tv_nsec - start.tv_nsec) / 1e9;

    printf("\n=== Final Results ===\n");
    double actual_total = 0.0;
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("  Account %d: $%.2f (%d transactions)\n",
               i, accounts[i].balance, accounts[i].transaction_count);
        actual_total += accounts[i].balance;
    }

    printf("\nInitial total: $%.2f\n", initial_total);
    printf("Actual total:  $%.2f\n", actual_total);
    printf("Difference:    $%.2f\n", actual_total - initial_total);
    printf("Time: %.6f seconds\n", elapsed);

    if (actual_total != initial_total) {
        printf("\nRACE CONDITION DETECTED!\n");
        printf("Run the program multiple times to observe different results.\n");
    } else {
        printf("\nNo race happened this run. Run it again a few times.\n");
    }

    return 0;
}
