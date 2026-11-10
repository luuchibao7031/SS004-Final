#pragma once

#include "Board.h"
#include "Piece.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX   
#endif
#include <windows.h>
#endif

class Renderer {
public:
    // Core draw function (TV1): ve ban co + sidebar Score/Next/Level
    // TV4: them combo + lastClearLines de hien thi dong "Combo xN <kieu xoa>" o sidebar
    static void draw(const Board& board, const Piece& activePiece,
                     int pieceX, int pieceY, int score, int level,
                     const Piece& nextPiece = Piece(),
                     int combo = 0, int lastClearLines = 0);

    // Khoi tao console: UTF-8, an cursor, bat VT processing
    static void initConsole();

    // To mau console bang windows.h SetConsoleTextAttribute
    static void setTextColor(unsigned short color);

    // Dat lai mau mac dinh
    static void resetColor();
};