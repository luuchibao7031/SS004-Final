#include "Board.h"

Board::Board() {
    for (int row = 0; row < Height; ++row) {
        for (int column = 0; column < Width; ++column) {
            const bool isWall = row == 0 || row == Height - 1 ||
                                column == 0 || column == Width - 1;
            cells_[row][column] = isWall ? '#' : ' ';
        }
    }
}

bool Board::canPlace(const Piece& piece, int x, int y) const {
    for (int row = 0; row < Piece::Size; ++row) {
        for (int column = 0; column < Piece::Size; ++column) {
            if (piece.cell(row, column) == ' ') {
                continue;
            }

            const int boardX = x + column;
            const int boardY = y + row;
            if (boardX < 1 || boardX >= Width - 1 ||
                boardY < 1 || boardY >= Height - 1 ||
                cells_[boardY][boardX] != ' ') {
                return false;
            }
        }
    }
    return true;
}

int Board::dropDistance(const Piece& piece, int x, int y) const {
    int distance = 0;
    while (canPlace(piece, x, y + distance + 1)) {
        ++distance;
    }
    return distance;
}

void Board::lock(const Piece& piece, int x, int y) {
    for (int row = 0; row < Piece::Size; ++row) {
        for (int column = 0; column < Piece::Size; ++column) {
            if (piece.cell(row, column) != ' ') {
                cells_[y + row][x + column] = piece.cell(row, column);
            }
        }
    }
}

int Board::clearCompletedLines() {
    int clearedLines = 0;
    for (int row = Height - 2; row > 0; --row) {
        bool complete = true;
        for (int column = 1; column < Width - 1; ++column) {
            if (cells_[row][column] == ' ') {
                complete = false;
                break;
            }
        }
        if (!complete) {
            continue;
        }

        for (int copyRow = row; copyRow > 1; --copyRow) {
            cells_[copyRow] = cells_[copyRow - 1];
        }
        for (int column = 1; column < Width - 1; ++column) {
            cells_[1][column] = ' ';
        }
        ++clearedLines;
        ++row;
    }
    return clearedLines;
}

char Board::at(int row, int column) const {
    return cells_[row][column];
}
