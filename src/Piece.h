#pragma once

#include <array>
#include <cstddef>

class Piece {
public:
    static constexpr int Size = 4;
    using Grid = std::array<std::array<char, Size>, Size>;

    explicit Piece(char type = 'O');

    char cell(int row, int column) const;
    Piece rotatedClockwise() const;
    char type() const;
    static Piece random();

private:
    Piece(char type, const Grid& cells);

    char type_;
    Grid cells_;
};