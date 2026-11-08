#include "Renderer.h"

#include <iostream>
#include <string>

void Renderer::draw(const Board& board, const Piece& activePiece,
                    int pieceX, int pieceY, int score, int level) {
    static bool firstFrame = true;
    std::string frame;
    frame.reserve((Board::Width + 1) * Board::Height + 64);

    if (firstFrame) {
        frame += "\x1b[2J";
        firstFrame = false;
    }
    frame += "\x1b[H";

    int ghostY = pieceY;
    while (board.canPlace(activePiece, pieceX, ghostY + 1)) {
        ++ghostY;
    }

    for (int row = 0; row < Board::Height; ++row) {
        for (int column = 0; column < Board::Width; ++column) {
            char cell = board.at(row, column);
            const int pieceRow = row - pieceY;
            const int pieceColumn = column - pieceX;
            const int ghostRow = row - ghostY;
            if (pieceRow >= 0 && pieceRow < Piece::Size &&
                pieceColumn >= 0 && pieceColumn < Piece::Size &&
                activePiece.cell(pieceRow, pieceColumn) != ' ') {
                cell = activePiece.cell(pieceRow, pieceColumn);
            } else if (ghostRow >= 0 && ghostRow < Piece::Size &&
                       pieceColumn >= 0 && pieceColumn < Piece::Size &&
                       activePiece.cell(ghostRow, pieceColumn) != ' ' &&
                       cell == ' ') {
                cell = '.';
            }
            frame += cell;
        }
        frame += '\n';
    }
    frame += "A/D: move  X: drop  W: rotate  Q: quit\n";
    frame += "Score: " + std::to_string(score) +
             "  Level: " + std::to_string(level) + '\n';
    std::cout << frame << std::flush;
}
