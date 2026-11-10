#include "Renderer.h"

#include <iostream>
#include <string>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace {

// =====================================================================
//  Palette mapped 100% from educ8s Python-Tetris (colors.py)
// =====================================================================
const std::string winBg     = "\x1b[48;2;44;44;127m";  // dark_blue = (44, 44, 127)
const std::string cellBg    = "\x1b[48;2;26;31;40m";   // dark_grey = (26, 31, 40)
const std::string dotCol    = "\x1b[48;2;26;31;40m\x1b[38;2;52;64;96m"; // subtle grid dot
const std::string borderCol = "\x1b[48;2;26;31;40m\x1b[38;2;85;110;165m";
const std::string ghostCol  = "\x1b[48;2;26;31;40m\x1b[38;2;95;120;175m"; // hollow ghost square
const std::string cardBg    = "\x1b[48;2;59;85;162m\x1b[97;1m"; // light_blue = (59, 85, 162)
const std::string cardBrd   = "\x1b[48;2;44;44;127m\x1b[38;2;95;135;225m";
const std::string textWhite = "\x1b[48;2;44;44;127m\x1b[97;1m"; // white = (255, 255, 255)
const std::string textDim   = "\x1b[48;2;44;44;127m\x1b[38;2;165;185;235m";
const std::string reset     = "\x1b[0m";

// Exact Tetromino colors from colors.py. Square tile '█▊ ': a full block
// plus a 3/4 block span 1.75 terminal columns over one text row - calibrated
// by eye on the actual terminal font so each mino renders as a true 1:1
// square. The remaining gap keeps neighbouring minos close, never touching.
const std::string getPieceBlock(char cell) {
    switch (cell) {
    case 'I': return cellBg + "\x1b[38;2;21;204;209m█▊ "; // cyan (21, 204, 209)
    case 'J': return cellBg + "\x1b[38;2;13;64;216m█▊ ";   // blue (13, 64, 216)
    case 'L': return cellBg + "\x1b[38;2;226;116;17m█▊ "; // orange (226, 116, 17)
    case 'O': return cellBg + "\x1b[38;2;237;234;4m█▊ ";   // yellow (237, 234, 4)
    case 'S': return cellBg + "\x1b[38;2;47;230;23m█▊ ";   // green (47, 230, 23)
    case 'T': return cellBg + "\x1b[38;2;200;156;255m█▊ ";   // purple #c89cff (200, 156, 255)
    case 'Z': return cellBg + "\x1b[38;2;232;18;18m█▊ ";   // red (232, 18, 18)
    default:  return "";
    }
}

// Next block in Sidebar (drawn on cardBg)
const std::string getNextCardBlock(char cell) {
    switch (cell) {
    case 'I': return cardBg + "\x1b[38;2;21;204;209m█▊ ";
    case 'J': return cardBg + "\x1b[38;2;13;64;216m█▊ ";
    case 'L': return cardBg + "\x1b[38;2;226;116;17m█▊ ";
    case 'O': return cardBg + "\x1b[38;2;237;234;4m█▊ ";
    case 'S': return cardBg + "\x1b[38;2;47;230;23m█▊ ";
    case 'T': return cardBg + "\x1b[38;2;200;156;255m█▊ ";
    case 'Z': return cardBg + "\x1b[38;2;232;18;18m█▊ ";
    default:  return cardBg + "   ";
    }
}

std::string centerText(const std::string& s, int totalWidth) {
    int len = static_cast<int>(s.length());
    if (len >= totalWidth) return s.substr(0, totalWidth);
    int lp = (totalWidth - len) / 2;
    int rp = totalWidth - len - lp;
    return std::string(lp, ' ') + s + std::string(rp, ' ');
}

void appendSidebarLine(std::string& out, int row, int score, int level, const Piece& nextPiece,
                       int combo, int lastClearLines) {
    // Keep sidebar labels centered in the 24-column card layout.
    switch (row) {
    case 1:
        out += textWhite; out += centerText("Score", 24); out += reset;
        break;
    case 2:
        out += winBg; out += "    "; out += cardBrd; out += "╭──────────────╮"; out += winBg; out += "    "; out += reset;
        break;
    case 3:
        out += winBg; out += "    "; out += cardBrd; out += "│";
        out += cardBg; out += centerText(std::to_string(score), 14);
        out += cardBrd; out += "│"; out += winBg; out += "    "; out += reset;
        break;
    case 4:
        out += winBg; out += "    "; out += cardBrd; out += "╰──────────────╯"; out += winBg; out += "    "; out += reset;
        break;
    case 6:
        out += textWhite; out += centerText("Next", 24); out += reset;
        break;
    case 7:
        out += winBg; out += "    "; out += cardBrd; out += "╭──────────────╮"; out += winBg; out += "    "; out += reset;
        break;
    case 8:
    case 9:
    case 10: {
        const int pr = row - 8;
        out += winBg; out += "    "; out += cardBrd; out += "│";
        out += cardBg; out += " ";
        for (int c = 0; c < Piece::Size; ++c) {
            char cell = nextPiece.cell(pr, c);
            if (cell != ' ') {
                out += getNextCardBlock(cell);
            } else {
                out += cardBg; out += "   ";
            }
        }
        out += cardBg; out += " ";
        out += cardBrd; out += "│"; out += winBg; out += "    "; out += reset;
        break;
    }
    case 11:
        out += winBg; out += "    "; out += cardBrd; out += "╰──────────────╯"; out += winBg; out += "    "; out += reset;
        break;
    case 13:
        out += textWhite; out += centerText("Level", 24); out += reset;
        break;
    case 14:
        out += winBg; out += "    "; out += cardBrd; out += "╭──────────────╮"; out += winBg; out += "    "; out += reset;
        break;
    case 15:
        out += winBg; out += "    "; out += cardBrd; out += "│";
        out += cardBg; out += centerText(std::to_string(level), 14);
        out += cardBrd; out += "│"; out += winBg; out += "    "; out += reset;
        break;
    case 16:
        out += winBg; out += "    "; out += cardBrd; out += "╰──────────────╯"; out += winBg; out += "    "; out += reset;
        break;
    case 17: {
        // TV4: dong combo + kieu xoa gan nhat, vd "Combo x2 Double" (dung 24 ky tu)
        static const char* const clearNames[] = {"", "Single", "Double", "Triple", "Tetris"};
        std::string status = "Combo: --";
        if (combo > 0) {
            status = "Combo x" + std::to_string(combo);
            if (lastClearLines >= 1 && lastClearLines <= 4) {
                status += std::string(" ") + clearNames[lastClearLines];
            }
        }
        out += (combo > 0 ? textWhite : textDim);
        out += centerText(status, 24);
        out += reset;
        break;
    }
    case 18:
        out += textDim; out += centerText("A/D:Move  W:Rotate", 24); out += reset;
        break;
    case 19:
        out += textDim; out += centerText("X:Drop    Q:Quit", 24); out += reset;
        break;
    default:
        out += winBg; out += std::string(24, ' '); out += reset;
        break;
    }
}

}  // namespace

void Renderer::initConsole() {
#ifdef _WIN32
    static bool initialized = false;
    if (initialized) {
        return;
    }
    initialized = true;

    // 1. Enable UTF-8 encoding for sharp box-drawing characters
    SetConsoleOutputCP(CP_UTF8);

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hIn  = GetStdHandle(STD_INPUT_HANDLE);

    // 2. Configure output console mode
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            dwMode |= DISABLE_NEWLINE_AUTO_RETURN;
            SetConsoleMode(hOut, dwMode);
        }

        // Hide blinking console cursor to eliminate flickering
        CONSOLE_CURSOR_INFO cursorInfo = { sizeof(cursorInfo), FALSE };
        SetConsoleCursorInfo(hOut, &cursorInfo);

        // Ensure console buffer is at least 80 cols x 24 rows
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (GetConsoleScreenBufferInfo(hOut, &csbi)) {
            SHORT targetCols = 80;
            SHORT targetRows = 24;
            if (csbi.dwSize.X < targetCols || csbi.dwSize.Y < targetRows) {
                COORD newSize = {
                    (csbi.dwSize.X < targetCols) ? targetCols : csbi.dwSize.X,
                    (csbi.dwSize.Y < targetRows) ? targetRows : csbi.dwSize.Y
                };
                SetConsoleScreenBufferSize(hOut, newSize);
            }
        }

        SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    }

    // 3. Disable QuickEdit and Echo on stdin so keyboard input doesn't move or pause console
    if (hIn != INVALID_HANDLE_VALUE) {
        DWORD modeIn = 0;
        if (GetConsoleMode(hIn, &modeIn)) {
            modeIn &= ~ENABLE_QUICK_EDIT_MODE;
            modeIn &= ~ENABLE_ECHO_INPUT;
            modeIn |= ENABLE_EXTENDED_FLAGS;
            SetConsoleMode(hIn, modeIn);
        }
    }

    std::cout << "\x1b[2J\x1b[?25l" << std::flush;
#else
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        std::cout << "\x1b[2J\x1b[?25l" << std::flush;
    }
#endif
}

void Renderer::setTextColor(unsigned short color) {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hOut, color);
#else
    (void)color;
#endif
}

void Renderer::resetColor() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#else
    std::cout << "\x1b[0m";
#endif
}

void Renderer::draw(const Board& board, const Piece& activePiece,
                    int pieceX, int pieceY, int score, int level,
                    const Piece& nextPiece, int combo, int lastClearLines) {
    initConsole();

    // Prepare full frame in memory buffer for zero-flicker rendering
    std::string frame;
    frame.reserve(4096);
    frame += "\x1b[H"; // Instantly return cursor to home (0, 0)

    const int ghostY = pieceY + board.dropDistance(activePiece, pieceX, pieceY);
    const int numPlayableCols = Board::Width - 2; // 13 columns (col 1..13)

    // Exact 20 rows layout (row 0 to 19): fits any console without scrolling
    for (int row = 0; row < Board::Height; ++row) {
        // Outer Left Margin (2 spaces with winBg)
        frame += winBg;
        frame += "  ";

        // 1. Draw Playfield
        if (row == 0) {
            // Top border: ┌ + 15 * ─── + ┐ (exactly 47 characters)
            frame += borderCol;
            frame += "┌";
            for (int c = 0; c < numPlayableCols; ++c) frame += "───";
            frame += "┐";
            frame += reset;
        } else if (row == Board::Height - 1) {
            // Bottom border: └ + 15 * ─── + ┘ (exactly 47 characters)
            frame += borderCol;
            frame += "└";
            for (int c = 0; c < numPlayableCols; ++c) frame += "───";
            frame += "┘";
            frame += reset;
        } else {
            // Content row: │ + 15 cells (3 cols each) + │ (exactly 47 characters)
            frame += borderCol;
            frame += "│";
            frame += reset;

            for (int col = 1; col < Board::Width - 1; ++col) {
                char cell = board.at(row, col);
                const int pieceRow = row - pieceY;
                const int pieceCol = col - pieceX;
                const int ghostRow = row - ghostY;

                if (pieceRow >= 0 && pieceRow < Piece::Size &&
                    pieceCol >= 0 && pieceCol < Piece::Size &&
                    activePiece.cell(pieceRow, pieceCol) != ' ') {
                    cell = activePiece.cell(pieceRow, pieceCol);
                } else if (ghostY != pieceY &&
                           ghostRow >= 0 && ghostRow < Piece::Size &&
                           pieceCol >= 0 && pieceCol < Piece::Size &&
                           activePiece.cell(ghostRow, pieceCol) != ' ' &&
                           cell == ' ') {
                    cell = '.';
                }

                if (cell == ' ') {
                    frame += dotCol;
                    frame += "·  ";
                    frame += reset;
                } else if (cell == '.') {
                    // Hollow square for ghost position
                    frame += ghostCol;
                    frame += "□□ ";
                    frame += reset;
                } else {
                    // Solid bordered square tile for active and locked pieces
                    frame += getPieceBlock(cell);
                    frame += reset;
                }
            }

            frame += borderCol;
            frame += "│";
            frame += reset;
        }

        // Padding between Playfield and Sidebar (4 spaces with winBg)
        frame += winBg;
        frame += "    ";

        // 2. Draw Sidebar Row (exactly 24 characters total width)
        appendSidebarLine(frame, row, score, level, nextPiece, combo, lastClearLines);

        // Outer Right Margin (2 spaces with winBg)
        frame += winBg;
        frame += "  ";
        frame += reset;

        // No trailing newline on the last row to prevent vertical scrolling
        if (row < Board::Height - 1) {
            frame += '\n';
        }
    }

    // Flush frame to console in one atomic operation
    std::cout << frame << std::flush;
}
