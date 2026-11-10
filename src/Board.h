#pragma once

#include "Piece.h"

#include <array>

class Board {
public:
    static constexpr int Height = 20;
    static constexpr int Width = 17;

    Board();

    // TV1 Core Module: Khoi tao ban co voi vien tuong '#' va o trong ' '
    void initBoard();

    bool canPlace(const Piece& piece, int x, int y) const;
    int dropDistance(const Piece& piece, int x, int y) const;
    void lock(const Piece& piece, int x, int y);
    int clearCompletedLines();
    char at(int row, int column) const;

private:
    std::array<std::array<char, Width>, Height> cells_;
};
