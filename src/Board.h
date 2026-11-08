#pragma once

#include "Piece.h"

#include <array>
#include <vector>

class Board {
public:
    static constexpr int Height = 20;
    static constexpr int Width = 15;

    Board();

    bool canPlace(const Piece& piece, int x, int y) const;
    void lock(const Piece& piece, int x, int y);
    int clearCompletedLines();
    char at(int row, int column) const;

private:
    std::array<std::array<char, Width>, Height> cells_;
};
