#include "Line.h"
#include "Board.h"

int removeLine(Board& board) {
    int clearedLines = 0;

    // Chỉ xử lý khu vực chơi, không đụng vào tường.
    for (int row = Board::Height - 2; row >= 1; --row) {
        bool full = true;

        for (int col = 1; col < Board::Width - 1; ++col) {
            if (board.cells_[row][col] == ' ') {
                full = false;
                break;
            }
        }

        if (!full) continue;

        // Kéo các hàng phía trên xuống.
        for (int k = row; k > 1; --k) {
            board.cells_[k] = board.cells_[k - 1];
        }

        // Làm trống hàng trên cùng.
        for (int col = 1; col < Board::Width - 1; ++col) {
            board.cells_[1][col] = ' ';
        }

        ++clearedLines;

        // Kiểm tra lại vị trí vì hàng phía trên vừa được kéo xuống.
        ++row;
    }

    return clearedLines;
}
