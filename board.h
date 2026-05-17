#ifndef BOARD_H
#define BOARD_H

struct GameState {
    char board[8][8];
    int current_turn; // 1 for player 'b', 2 for player 'c'
    int game_status; // 0 - during the game, 1 - winner 'b', 2 - winner 'c'
};

void printBoard(char board[8][8]);
void initBoard(char board[8][8]);
int movePiece(char board[8][8], char from[], char to[], int player_id);
int checkWin(char board[8][8]);

#endif