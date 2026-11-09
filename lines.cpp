const int ROWS = 20, COLS = 10;

void removeLine(int board[ROWS][COLS], int &score) {
    for (int r = ROWS - 1; r >= 0; r--) {
        bool full = true;
        for (int c = 0; c < COLS; c++)
            if (board[r][c] == 0) { full = false; break; }
        if (full) {
            for (int k = r; k > 0; k--)
                for (int c = 0; c < COLS; c++)
                    board[k][c] = board[k - 1][c];
            for (int c = 0; c < COLS; c++) board[0][c] = 0;
            score += 100;
            r++;
        }
    }
}