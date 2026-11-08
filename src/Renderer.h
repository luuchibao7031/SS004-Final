#pragma once

#include "Board.h"
#include "Piece.h"

class Renderer {
public:
    static void draw(const Board& board, const Piece& activePiece,
                     int pieceX, int pieceY, int score, int level);
};
