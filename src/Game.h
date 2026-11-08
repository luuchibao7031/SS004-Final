#pragma once

#include "Board.h"
#include "Piece.h"
#include "Score.h"

class Game {
public:
    Game();
    void run();

private:
    bool handleCommand();
    bool canMove(int deltaX, int deltaY) const;
    bool tryRotate();
    void lockActivePiece();
    void spawnPiece();

    Board board_;
    Piece activePiece_;
    Score score_;
    int pieceX_;
    int pieceY_;
};
