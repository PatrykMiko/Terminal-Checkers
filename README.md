# Terminal Checkers (C / POSIX IPC)

A two-player checkers game played directly in the Linux terminal. This project serves as a practical demonstration of advanced Inter-Process Communication (IPC) and multithreading in C within a POSIX environment. 

The application launches in a single terminal and forks into two independent processes representing Player 1 and Player 2. The players can play the game and chat with each other in real-time asynchronously.

## Features

* **Multiprocessing (`fork`):** The game runs as two independent processes interacting in the same terminal.
* **Shared State (POSIX Shared Memory):** The game board and turn data are kept in shared memory (`shm_open`, `mmap`), making changes instantly visible to both processes.
* **Turn Synchronization (Named Semaphores):** Ensures strict turn-based gameplay using `sem_open` and `sem_wait`.
* **Asynchronous Chat (Message Queues & `pthreads`):** Players can send and receive chat messages at any time without blocking their turn, thanks to background listening threads and System V Message Queues.
* **UI Protection (Mutexes):** The terminal output is protected by `pthread_mutex_lock` to prevent chat messages from interrupting the drawing of the board or input prompts.
* **Graceful Exit (Signals):** Captures `SIGINT` (Ctrl+C) to safely clean up system IPC resources (unlinking semaphores, shared memory, and destroying message queues) before exiting.

## Prerequisites

To compile and run this project, you need:
* A Linux/Unix-based operating system (POSIX compliant).
* `gcc` compiler.

## Compilation and Execution

1. Clone the repository and navigate to the project directory.
2. Compile the source code using the following command (make sure to link the pthread library):
   ```bash
   gcc main.c board.c -o warcaby -pthread
   ```

Starting the program: ./warcaby
