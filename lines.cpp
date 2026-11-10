#include "Line.h"

#include "Board.h"

int removeLine(Board& board) {
    int clearedLines = 0;

    // Row 0 and Height - 1 are permanent walls. Only scan playable rows.
    for (int row = Board::Height - 2; row >= 1; --row) {
        bool full = true;

        for (int column = 1; column < Board::Width - 1; ++column) {
            if (board.cells_[row][column] == ' ') {
                full = false;
                break;
            }
        }

        if (!full) continue;

        // Pull every playable row above the completed row down by one.
        for (int copyRow = row; copyRow > 1; --copyRow) {
            board.cells_[copyRow] = board.cells_[copyRow - 1];
        }

        // Clear the top playable row.
        for (int column = 1; column < Board::Width - 1; ++column) {
            board.cells_[1][column] = ' ';
        }

        ++clearedLines;
        ++row; // Re-check this index after shifting.
    }

    return clearedLines;
}
