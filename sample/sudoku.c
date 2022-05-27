#include <stdio.h>

typedef enum A {
    TRUE = 1,
    FALSE = 0
} BOOL;

static int board[9][9] = {
    {5, 3, 0, 0, 7, 0, 0, 0, 0},
    {6, 0, 0, 1, 9, 5, 0, 0, 0},
    {0, 9, 8, 0, 0, 0, 0, 6, 0},
    {0, 0, 0, 0, 6, 0, 0, 0, 3},
    {0, 0, 0, 8, 0, 3, 0, 0, 1},
    {7, 0, 0, 0, 0, 0, 0, 0, 6},
    {0, 6, 0, 0, 0, 0, 2, 8, 0},
    {0, 0, 0, 4, 1, 9, 0, 0, 5},
    {0, 0, 0, 0, 8, 0, 0, 7, 9}};

static int answer = 0;

void showBoard(void);
BOOL putNumber(int, int, int);
BOOL checkNumber(int, int, int);
void start(void);

void showBoard(void) {
    int i, j;

    printf("%u個目の解答です\n", answer);

    for (j = 0; j < 9; j++) {
        for (i = 0; i < 9; i++) {
            printf("%d ", board[j][i]);
        }
        printf("\n");
    }
    printf("\n");
}

BOOL checkNumber(int i, int j, int number) {
    int x, y;
    int bi, bj;

    for (x = 0; x < 9; x++) {
        if (board[j][x] == number) {
            return FALSE;
        }
    }

    for (y = 0; y < 9; y++) {
        if (board[y][i] == number) {
            return FALSE;
        }
    }

    bi = i / 3 * 3;
    bj = j / 3 * 3;

    for (y = 0; y < 3; y++) {
        for (x = 0; x < 3; x++) {
            if (board[bj + y][bi + x] == number) {
                return FALSE;
            }
        }
    }

    return TRUE;
}

BOOL putNumber(int i, int j, int number) {
    int fix_flag = FALSE;

    if (board[j][i] != number) {
        if (board[j][i] != 0) {
            return FALSE;
        }

        if (!checkNumber(i, j, number)) {
            return FALSE;
        }

        board[j][i] = number;
    } else {
        fix_flag = TRUE;
    }

    if (i == 9 - 1 && j == 9 - 1) {
        answer++;

        showBoard();
    } else {
        int n;
        int next_i, next_j;

        if (i + 1 >= 9) {
            next_i = 0;
            next_j = j + 1;
        } else {
            next_i = i + 1;
            next_j = j;
        }

        for (n = 1; n <= 9; n++) {
            putNumber(next_i, next_j, n);
        }
    }

    if (!fix_flag) {
        board[j][i] = 0;
    }

    return TRUE;
}

void start(void) {
    int n;

    for (n = 1; n <= 9; n++) {
        putNumber(0, 0, n);
    }
}

int main(void) {
    start();

    printf("回答数：%u\n", answer);
    return 0;
}