#pragma once

#include "Board.h"
#include "Piece.h"
#include "Score.h"

class Game {
public:
    Game();
    void run();

private:
    bool handleCommand(bool& stateChanged);
    bool canMove(int deltaX, int deltaY) const;
    bool tryRotate();
    void lockActivePiece();
    void spawnPiece();

    Board board_;
    Piece activePiece_;
    Piece nextPiece_;   // TV1: luu khoi tiep theo de hien thi trong Sidebar
    Score score_;
    int pieceX_;
    int pieceY_;
};
