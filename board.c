#include <stdio.h>
#include "board.h"

// initial board
void initBoard(char board[8][8]) {
    for(int i = 0; i < 8; i++) {
        for(int j = 0; j < 8; j++) {
            if ((i == 0 && j % 2 == 1) || (i == 1 && j % 2 == 0) || (i == 2 && j % 2 == 1)) {
                board[i][j] = 'c';
            }
            else if ((i == 5 && j % 2 == 0) || (i == 6 && j % 2 == 1) || (i == 7 && j % 2 == 0)) {
                board[i][j] = 'b';
            }
            else {
                board[i][j] = ' ';
            }
        }
    }
}

// showing the board
void printBoard(char board[8][8]) {
    printf("\n");
    printf("    A   B   C   D   E   F   G   H\n");
    printf("  +---+---+---+---+---+---+---+---+\n");
    for (int i = 0; i < 8; i++) {
        printf("%d |", i + 1);
        for (int j = 0; j < 8; j++) {
            printf(" %c |", board[i][j]);
        }
        printf("\n  +---+---+---+---+---+---+---+---+\n");
    }
    printf("\n");
}

// making a move
int movePiece(char board[8][8], char from[], char to[], int player_id) {
    int col_from = from[0] - 'A';
    int row_from = from[1] - '1';

    int col_to = to[0] - 'A';
    int row_to = to[1] - '1';

    // protection against going outside the board
    if (col_from < 0 || col_from > 7 || row_from < 0 || row_from > 7 || col_to < 0 || col_to > 7 || row_to < 0 || row_to > 7) {
        printf("Błędne współrzędne!\n");
        return 0;
    }

    char my_piece = (player_id == 1) ? 'b' : 'c';
    char my_king = (player_id == 1) ? 'B' : 'C';
    char opp_piece = (player_id == 1) ? 'c' : 'b';
    char opp_king = (player_id == 1) ? 'C' : 'B';

    char current_piece = board[row_from][col_from];

    // checking if it is mine pawn
    if (current_piece != my_piece && current_piece != my_king) {
        printf("To nie jest Twój pionek albo pole jest puste\n");
        return 0;
    }

    // checking if the field is empty
    if (board[row_to][col_to] != ' ') {
        printf("Pole docelowe jest zajęte!\n");
        return 0;
    }

    int col_diff = col_to - col_from;
    int row_diff = row_to - row_from;

    int abs_col_diff = (col_diff > 0) ? col_diff : -col_diff;
    int abs_row_diff = (row_diff > 0) ? row_diff : -row_diff;

    int is_king = (current_piece == my_king);

    // regular move one square
    if (abs_col_diff == 1 && abs_row_diff == 1) {
        // checking the direction of the rows - ONLY for regular pawns
        if (!is_king) {
            if ((player_id == 1 && row_diff != -1) || (player_id == 2 && row_diff != 1)) {
                printf("Zwykły pionek może poruszać się tylko do przodu po skosie.\n");
                return 0;
            }
        }

        board[row_to][col_to] = current_piece;
        board[row_from][col_from] = ' ';
    }

    // beating the opponent by two squares
    else if (abs_col_diff == 2 && abs_row_diff == 2) {
        // checking if beating is in right direction - for regular pawns
        if (!is_king) {
            if ((player_id == 1 && row_diff != -2) || (player_id == 2 && row_diff != 2)) {
                printf("Zwykły pionek może bić tylko do przodu po skosie!\n");
                return 0;
            }
        }

        int jumped_row = (row_from + row_to) / 2;
        int jumped_col = (col_from + col_to) / 2;
        char jumped_piece = board[jumped_row][jumped_col];

        // checking if there is an opponent pawn/king and beating it
        if (jumped_piece == opp_piece || jumped_piece == opp_king) {
            board[row_to][col_to] = current_piece;
            board[row_from][col_from] = ' ';
            board[jumped_row][jumped_col] = ' ';
            printf("Bicie!\n");
        } else {
            printf("Na Twojej drodze nie ma pionka przeciwnika!\n");
            return 0;
        }
    }
    // incorrect movement range
    else {
        printf("Nieprawidłowy zasięg ruchu\n");
        return 0;
    }

    // prootion to king - if regular pawn reaches the opposite end of the board
    if (player_id == 1 && row_to == 0 && board[row_to][col_to] == 'b') {
        board[row_to][col_to] = 'B';
        printf("Twój pionek staje się damką\n");
    } else if (player_id == 2 && row_to == 7 && board[row_to][col_to] == 'c') {
        board[row_to][col_to] = 'C';
        printf("Twój pionek staje się damką\n");
    }

    return 1;
}

// checking if someone win
int checkWin(char board[8][8]) {
    int count_b = 0;
    int count_c = 0;

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j] == 'b' || board[i][j] == 'B') {
                count_b++;
            } else if (board[i][j] == 'c' || board[i][j] == 'C') {
                count_c++;
            }
        }
    }

    // if there are zero b pawns, the winner is the player 2
    if (count_b == 0) return 2;

    // if there are zero c pawns, the winner is the player 1
    if (count_c == 0) return 1;

    // if both players have pawns the game is continued
    return 0;
}