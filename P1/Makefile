CC = gcc
CFLAGS = -Wall -Wextra -pthread

all: phase1 phase2 phase3 phase4

phase1: phase1.c
	$(CC) $(CFLAGS) phase1.c -o phase1

phase2: phase2.c
	$(CC) $(CFLAGS) phase2.c -o phase2

phase3: phase3.c
	$(CC) $(CFLAGS) phase3.c -o phase3

phase4: phase4.c
	$(CC) $(CFLAGS) phase4.c -o phase4

clean:
	rm -f phase1 phase2 phase3 phase4
