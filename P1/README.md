# CS 3502 Project 1 - Multi-Threaded Banking System

## Overview

This project demonstrates key concepts in concurrent programming using POSIX threads in C.  
The system simulates a simple banking environment where multiple teller threads perform deposits, withdrawals, and transfers on shared bank accounts.

The project is divided into four phases to illustrate common synchronization issues and their solutions.

---

## Project Phases

### Phase 1 - Race Condition Demonstration
Multiple teller threads perform deposits and withdrawals on shared account balances without synchronization.

This phase demonstrates:
- Race conditions
- Non-deterministic results
- Data corruption caused by concurrent access

Running the program multiple times may produce different totals due to unsynchronized access.

---

### Phase 2 - Mutex Protection
This phase introduces mutex locks to protect shared account data.

Each account has an associated mutex that ensures:
- Only one thread modifies an account at a time
- Data consistency is maintained
- Race conditions are eliminated

The final balance should always match the expected total.

---

### Phase 3 - Deadlock Demonstration
This phase introduces account transfers between two accounts.

Two threads attempt to transfer money in opposite directions while locking accounts in different orders. This can cause a deadlock where each thread holds one lock and waits for the other.

The program detects lack of progress and reports a suspected deadlock.

---

### Phase 4 - Deadlock Resolution
Deadlock is prevented by enforcing a consistent lock ordering strategy.

Threads always lock the lower account ID first and the higher account ID second.  
This eliminates circular wait and prevents deadlock.

---

## File Structure
├── phase1.c
├── phase2.c
├── phase3.c
├── phase4.c
├── Makefile
├── README.md


---

## Compilation

You can compile all phases using the Makefile:

make

Or compile individually:
gcc -Wall -Wextra -pthread phase1.c -o phase1
gcc -Wall -Wextra -pthread phase2.c -o phase2
gcc -Wall -Wextra -pthread phase3.c -o phase3
gcc -Wall -Wextra -pthread pahse4.c -o phase4


---

## Running the Programs
./phase1
./phase2
./phase3
./phase4


Phase 1 should demonstrate race conditions.  
Phase 2 should show correct synchronized behavior.  
Phase 3 demonstrates deadlock.  
Phase 4 resolves deadlock using lock ordering.

---

## Key Concepts Demonstrated

- POSIX threads (`pthread`)
- Race conditions
- Mutex synchronization
- Deadlock conditions
- Deadlock prevention through lock ordering
- Concurrent program design

---

## Author

Ethan Gregg  
CS 3502 - Operating Systems
