#include "Piece.h"

#include <array>
#include <cstdlib>

namespace {

using Grid = Piece::Grid;

const std::array<Grid, 7> Templates = {{
    {{{' ', 'I', ' ', ' '}, {' ', 'I', ' ', ' '},
      {' ', 'I', ' ', ' '}, {' ', 'I', ' ', ' '}}},
    {{{' ', 'O', 'O', ' '}, {' ', 'O', 'O', ' '},
      {' ', ' ', ' ', ' '}, {' ', ' ', ' ', ' '}}},
    {{{' ', 'T', ' ', ' '}, {'T', 'T', 'T', ' '},
      {' ', ' ', ' ', ' '}, {' ', ' ', ' ', ' '}}},
    {{{' ', 'S', 'S', ' '}, {'S', 'S', ' ', ' '},
      {' ', ' ', ' ', ' '}, {' ', ' ', ' ', ' '}}},
    {{{'Z', 'Z', ' ', ' '}, {' ', 'Z', 'Z', ' '},
      {' ', ' ', ' ', ' '}, {' ', ' ', ' ', ' '}}},
    {{{'J', ' ', ' ', ' '}, {'J', 'J', 'J', ' '},
      {' ', ' ', ' ', ' '}, {' ', ' ', ' ', ' '}}},
    {{{' ', ' ', 'L', ' '}, {'L', 'L', 'L', ' '},
      {' ', ' ', ' ', ' '}, {' ', ' ', ' ', ' '}}}
}};

}  // namespace

Piece::Piece(char type) : type_(type), cells_{} {
    constexpr std::array<char, 7> types = {'I', 'O', 'T', 'S', 'Z', 'J', 'L'};
    for (std::size_t index = 0; index < types.size(); ++index) {
        if (types[index] == type) {
            cells_ = Templates[index];
            return;
        }
    }
    type_ = 'O';
    cells_ = Templates[1];
}

Piece::Piece(char type, const Grid& cells) : type_(type), cells_(cells) {}

char Piece::cell(int row, int column) const {
    return cells_[row][column];
}

Piece Piece::rotatedClockwise() const {
    Grid rotated{};
    for (int row = 0; row < Size; ++row) {
        for (int column = 0; column < Size; ++column) {
            rotated[row][column] = cells_[Size - 1 - column][row];
        }
    }
    return Piece(type_, rotated);
}

char Piece::type() const {
    return type_;
}

Piece Piece::random() {
    constexpr std::array<char, 7> types = {'I', 'O', 'T', 'S', 'Z', 'J', 'L'};
    return Piece(types[std::rand() % types.size()]);
}