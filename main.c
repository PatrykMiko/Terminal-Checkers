#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include "board.h"
#include <sys/wait.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <pthread.h>

int msgid = -1;
// padlock (mutex) to protect the screen from clutter
pthread_mutex_t console_mutex = PTHREAD_MUTEX_INITIALIZER;

// message structure
struct msgbuf {
    long mtype;       // to who (1 or 2)
    char mtext[256];  // the text
};

// a structure to pass data to a thread
struct ThreadArgs {
    int my_player_id;
};

// listening thread
void* chat_listener(void* arg) {
    struct ThreadArgs* args = (struct ThreadArgs*)arg;
    struct msgbuf received_msg;

    while (1) {
        //we are waiting for your message addressed to us
        if (msgrcv(msgid, &received_msg, sizeof(received_msg.mtext), args->my_player_id, 0) > 0) {

            pthread_mutex_lock(&console_mutex);

            printf("\n\n[Przeciwnik]: %s\n", received_msg.mtext);
            printf("Wpisz ruch (np. C3 D4) lub /wiadomosc: ");
            fflush(stdout);

            pthread_mutex_unlock(&console_mutex);
        }
    }
    return NULL;
}

void cleanup_handler(int sig) {
    printf("\n Przechwycono sygnał (Ctrl+C). Sprzątanie zasobów.\n");

    // Removing shared memory and semaphores from the system
    shm_unlink("/checkers_shm");
    sem_unlink("/sem_checkers_p1");
    sem_unlink("/sem_checkers_p2");
    if (msgid != -1) {
        msgctl(msgid, IPC_RMID, NULL);
    }
    exit(0);
}

int main(int argc, char *argv[]) {

    // signal handling, when ctrl-c is pressed, the system clears resources and closes the program
    signal(SIGINT, cleanup_handler);

    // creating a message queue
    msgid = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
    if (msgid == -1) {
        printf("Błąd tworzenia kolejki komunikatów\n");
        return 1;
    }

    // shared memory
    int shm_fd = shm_open("/checkers_shm", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(struct GameState));

    struct GameState *state = mmap(NULL, sizeof(struct GameState), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // Player 1 starts, so their semaphore is 1
    sem_t *sem_p1 = sem_open("/sem_checkers_p1", O_CREAT, 0666, 1);
    // Player 2 waits, so their semaphore is 0
    sem_t *sem_p2 = sem_open("/sem_checkers_p2", O_CREAT, 0666, 0);

    // we clone the program into two separate processes
    pid_t pid = fork();
    int player_id;
    if (pid < 0) {
        printf("Błąd przy tworzeniu procesów\n");
        return 1;
    }
    else if (pid == 0) {
        // child process
        player_id = 2;
    }
    else {
        // parent process
        player_id = 1;
        // setting up the board
        initBoard(state->board);
        state->current_turn = 1;
        state->game_status = 0;
    }

    // from this point on, both processes follow the same path, but have different player_id!
    sem_t *my_sem = (player_id == 1) ? sem_p1 : sem_p2;
    sem_t *opp_sem = (player_id == 1) ? sem_p2 : sem_p1;

    // we start a listening thread for each player
    pthread_t listener_thread;
    struct ThreadArgs t_args;
    t_args.my_player_id = player_id;
    pthread_create(&listener_thread, NULL, chat_listener, &t_args);

    while (1) {
        // checking if it's my turn
        sem_wait(my_sem);

        // checking if the opponent win
        if (state->game_status != 0) {
            // lock the mutex before printing to the screen
            pthread_mutex_lock(&console_mutex);
            printf("\nKoniec gry! Wygrał gracz %d.\n", state->game_status);
            // unlock the mutex
            pthread_mutex_unlock(&console_mutex);
            break;
        }

        // lock the mutex before printing the board so the chat thread won't interrupt
        pthread_mutex_lock(&console_mutex);
        printf("  Kolej gracza: %d \n", player_id);
        printBoard(state->board);
        // unlock the mutex after drawing is done
        pthread_mutex_unlock(&console_mutex);

        // player movement
        int valid_move = 0;
        char input[256]; // buffer for reading whole line of text

        while (valid_move == 0) {
            // protect the prompt with mutex
            pthread_mutex_lock(&console_mutex);
            printf("Wpisz ruch (np. C3 D4) lub /wiadomosc: ");
            fflush(stdout); // force the text to appear on screen immediately
            pthread_mutex_unlock(&console_mutex);

            if (fgets(input, sizeof(input), stdin) == NULL) continue;
            input[strcspn(input, "\n")] = 0;

            // check if the first character is '/' (chat message)
            if (input[0] == '/') {
                struct msgbuf msg;
                msg.mtype = (player_id == 1) ? 2 : 1;
                strncpy(msg.mtext, input + 1, 255);
                msgsnd(msgid, &msg, sizeof(msg.mtext), 0);
            }
            // if it doesn't start with '/', treat it as a move
            else {
                char from[3], to[3];
                if (sscanf(input, "%2s %2s", from, to) == 2) {
                    valid_move = movePiece(state->board, from, to, player_id);
                } else {
                    pthread_mutex_lock(&console_mutex);
                    printf("Niepoprawny format! Uzyj 'C3 D4' lub '/wiadomosc'\n");
                    pthread_mutex_unlock(&console_mutex);
                }
            }
        }

        // checking the win
        state->game_status = checkWin(state->board);
        if (state->game_status != 0) {
            pthread_mutex_lock(&console_mutex);
            printBoard(state->board);
            printf("\nGratulacje, wygrałeś graczu %d!\n", player_id);
            pthread_mutex_unlock(&console_mutex);

            sem_post(opp_sem);
            break;
        }

        // change of turn
        state->current_turn = (player_id == 1) ? 2 : 1;
        // the opponent movement
        sem_post(opp_sem);
    }

    // waiting for all processes to complete
    if (pid > 0) {
        wait(NULL);
        printf("\nSprzątanie zasobów.\n");
        shm_unlink("/checkers_shm");
        sem_unlink("/sem_checkers_p1");
        sem_unlink("/sem_checkers_p2");
        msgctl(msgid, IPC_RMID, NULL);
    }

    return 0;
}
