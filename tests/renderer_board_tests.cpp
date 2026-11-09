#include "../src/Board.h"
#include "../src/Piece.h"
#include "../src/Renderer.h"

#include <cassert>
#include <iostream>

namespace {

void testInitBoardCreatesCorrectBoundaries() {
    Board board;

    // Verify boundary walls
    for (int col = 0; col < Board::Width; ++col) {
        assert(board.at(0, col) == '#');
        assert(board.at(Board::Height - 1, col) == '#');
    }
    for (int row = 0; row < Board::Height; ++row) {
        assert(board.at(row, 0) == '#');
        assert(board.at(row, Board::Width - 1) == '#');
    }

    // Verify inner empty playfield
    for (int row = 1; row < Board::Height - 1; ++row) {
        for (int col = 1; col < Board::Width - 1; ++col) {
            assert(board.at(row, col) == ' ');
        }
    }
}

void testInitBoardResetsBoard() {
    Board board;
    const Piece piece('O');

    // Place and lock a piece ('O' has cells at relative (0,1), (0,2), (1,1), (1,2))
    assert(board.canPlace(piece, 2, 2));
    board.lock(piece, 2, 2);
    assert(board.at(2, 3) == 'O');
    assert(board.at(3, 3) == 'O');

    // Re-initialize board
    board.initBoard();

    // Verify cells are reset back to empty spaces
    assert(board.at(2, 3) == ' ');
    assert(board.at(3, 3) == ' ');
}

void testRendererMethods() {
    Board board;
    const Piece active('T');
    const Piece next('I');

    // Test calling draw with nextPiece
    Renderer::draw(board, active, 5, 1, 1000, 2, next);

    // Test calling draw with default nextPiece parameter
    Renderer::draw(board, active, 5, 1, 500, 1);

    // Test resetColor
    Renderer::resetColor();
}

}  // namespace

int main() {
    testInitBoardCreatesCorrectBoundaries();
    testInitBoardResetsBoard();
    testRendererMethods();
    std::cout << "\nAll TV1 (Renderer & Board) unit tests passed successfully!\n";
    return 0;
}
