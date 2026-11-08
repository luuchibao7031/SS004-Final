#include "Renderer.h"

#include <cstdlib>
#include <iostream>

void Renderer::draw(const Board& board, const Piece& activePiece,
                    int pieceX, int pieceY, int score) {
    system("cls");
    for (int row = 0; row < Board::Height; ++row) {
        for (int column = 0; column < Board::Width; ++column) {
            char cell = board.at(row, column);
            const int pieceRow = row - pieceY;
            const int pieceColumn = column - pieceX;
            if (pieceRow >= 0 && pieceRow < Piece::Size &&
                pieceColumn >= 0 && pieceColumn < Piece::Size &&
                activePiece.cell(pieceRow, pieceColumn) != ' ') {
                cell = activePiece.cell(pieceRow, pieceColumn);
            }
            std::cout << cell;
        }
        std::cout << '\n';
    }
    std::cout << "A/D: move  X: drop  W: rotate  Q: quit\n";
    std::cout << "Score: " << score << '\n';
}
