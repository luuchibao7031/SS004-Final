#include "../src/Board.h"
#include "../src/Piece.h"

#include <cassert>

namespace {

void testTetrominoesUseFourByFourMatrices() {
    constexpr char types[] = {'I', 'O', 'T', 'S', 'Z', 'J', 'L'};
    static_assert(Piece::Size == 4);
    static_assert(Board::Width - 2 == 15);

    for (char type : types) {
        const Piece piece(type);
        int occupiedCells = 0;
        for (int row = 0; row < Piece::Size; ++row) {
            for (int column = 0; column < Piece::Size; ++column) {
                if (piece.cell(row, column) != ' ') {
                    ++occupiedCells;
                }
            }
        }
        assert(piece.type() == type);
        assert(occupiedCells == 4);
    }
}

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

void testDropDistanceStopsAtBottom() {
    const Board board;
    const Piece piece('O');

    assert(board.dropDistance(piece, 1, 1) == Board::Height - 4);
}

}  // namespace

int main() {
    testTetrominoesUseFourByFourMatrices();
    testPieceRotatesBackToOriginal();
    testBoardRejectsInvalidAndOccupiedPositions();
    testDropDistanceStopsAtBottom();
    return 0;
}
