#include "../src/Board.h"
#include "../src/Piece.h"

#include <cassert>

namespace {

void testPieceRotatesBackToOriginal() {
    const Piece original('T');
    Piece rotated = original;

    for (int rotation = 0; rotation < 4; ++rotation) {
        rotated = rotated.rotatedClockwise();
    }

    for (int row = 0; row < Piece::Size; ++row) {
        for (int column = 0; column < Piece::Size; ++column) {
            assert(rotated.cell(row, column) == original.cell(row, column));
        }
    }
}

void testBoardRejectsInvalidAndOccupiedPositions() {
    Board board;
    const Piece piece('O');

    assert(board.canPlace(piece, 1, 1));
    assert(!board.canPlace(piece, -1, 1));
    assert(!board.canPlace(piece, 1, Board::Height - 2));

    board.lock(piece, 1, 1);
    assert(!board.canPlace(piece, 1, 1));
}

}  // namespace

int main() {
    testPieceRotatesBackToOriginal();
    testBoardRejectsInvalidAndOccupiedPositions();
    return 0;
}
