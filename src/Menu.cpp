
// ============================================================================
// TV5 - TETRIS MAIN MENU
// Giữ nguyên thiết kế Menu ban đầu của TV5.
// Tích hợp với project SS004-Final.
// Return:
//   1 = Play
//   2 = High Score
//   3 = Exit
// ============================================================================

#include "Menu.h"

#include <algorithm>
#include <chrono>
#include <climits>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
    #include <conio.h>
#else
    #include <sys/select.h>
    #include <termios.h>
    #include <unistd.h>
#endif

using namespace std;

namespace {

// ============================================================================
// CONSTANTS
// ============================================================================

constexpr int KEY_UP = 1001;
constexpr int KEY_DOWN = 1002;
constexpr int KEY_LEFT = 1003;
constexpr int KEY_RIGHT = 1004;

constexpr int GW = 24;
constexpr int GH = 22;
constexpr int SW = GW * 2;
constexpr int ROWS = GH + 2;
constexpr int COLS = SW + 2;

// Original color palette.
constexpr int NAVY = 0x2a2d7a;
constexpr int BOARD = 0x1b1f2a;
constexpr int FRAME = 0x4a6aa8;
constexpr int DOT = 0x3b4358;

constexpr int GHOSTC = 0x6f8fd0;
constexpr int BOXB = 0x4a7de0;
constexpr int BOXF = 0x3a53a0;
constexpr int DIMT = 0xaab0e0;
constexpr int WHITE = 0xffffff;

constexpr int PC[7] = {
    0x18c8c8,
    0xe8e818,
    0xa010f0,
    0x2ee82e,
    0xe02020,
    0x4a5de0,
    0xe07818
};

// ============================================================================
// MENU TERMINAL
// Menu có bộ đọc phím riêng, không phụ thuộc Input.cpp của gameplay.
// ============================================================================

#ifdef _WIN32

class MenuTerminal {
public:
    MenuTerminal() {
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);

        HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);

        if (output != INVALID_HANDLE_VALUE &&
            GetConsoleMode(output, &oldMode_)) {

            if (SetConsoleMode(
                    output,
                    oldMode_ | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
                changedMode_ = true;
            }
        }

        cout << "\x1b[?25l\x1b[?7l" << flush;
    }

    ~MenuTerminal() {
        cout << "\x1b[0m\x1b[?7h\x1b[?25h\n" << flush;

        if (changedMode_) {
            SetConsoleMode(
                GetStdHandle(STD_OUTPUT_HANDLE),
                oldMode_
            );
        }
    }

private:
    DWORD oldMode_ = 0;
    bool changedMode_ = false;
};

int readMenuKey() {
    if (!_kbhit()) {
        return -1;
    }

    const int key = _getch();

    if (key == 0 || key == 224) {
        switch (_getch()) {
            case 72: return KEY_UP;
            case 80: return KEY_DOWN;
            case 75: return KEY_LEFT;
            case 77: return KEY_RIGHT;
            default: return -1;
        }
    }

    return key;
}

#else

class MenuTerminal {
public:
    MenuTerminal() {
        if (isatty(STDIN_FILENO) &&
            tcgetattr(STDIN_FILENO, &oldSettings_) == 0) {

            termios raw = oldSettings_;

            raw.c_lflag &= static_cast<tcflag_t>(
                ~(ICANON | ECHO)
            );

            raw.c_iflag &= static_cast<tcflag_t>(
                ~(IXON | ICRNL)
            );

            raw.c_cc[VMIN] = 0;
            raw.c_cc[VTIME] = 0;

            if (tcsetattr(
                    STDIN_FILENO,
                    TCSANOW,
                    &raw) == 0) {
                active_ = true;
            }
        }

        cout << "\x1b[?25l\x1b[?7l" << flush;
    }

    ~MenuTerminal() {
        cout << "\x1b[0m\x1b[?7h\x1b[?25h\n" << flush;

        if (active_) {
            tcsetattr(
                STDIN_FILENO,
                TCSANOW,
                &oldSettings_
            );
        }
    }

private:
    termios oldSettings_{};
    bool active_ = false;
};

bool readMenuByte(char& value, long timeoutMicroseconds) {
    fd_set inputSet;

    FD_ZERO(&inputSet);
    FD_SET(STDIN_FILENO, &inputSet);

    timeval timeout{};

    timeout.tv_sec = timeoutMicroseconds / 1000000;
    timeout.tv_usec = timeoutMicroseconds % 1000000;

    const int ready = select(
        STDIN_FILENO + 1,
        &inputSet,
        nullptr,
        nullptr,
        &timeout
    );

    if (ready <= 0) {
        return false;
    }

    return read(STDIN_FILENO, &value, 1) == 1;
}

int readMenuKey() {
    char key = 0;

    if (!readMenuByte(key, 0)) {
        return -1;
    }

    if (static_cast<unsigned char>(key) == 0x1B) {
        char bracket = 0;
        char direction = 0;

        if (readMenuByte(bracket, 12000) &&
            bracket == '[' &&
            readMenuByte(direction, 12000)) {

            switch (direction) {
                case 'A': return KEY_UP;
                case 'B': return KEY_DOWN;
                case 'D': return KEY_LEFT;
                case 'C': return KEY_RIGHT;
                default: return -1;
            }
        }

        return 27;
    }

    return static_cast<unsigned char>(key);
}

#endif

// ============================================================================
// GRAPHICS DATA
// ============================================================================

struct Px {
    string ch = " ";
    int fg = WHITE;
    int bg = NAVY;
};

using PieceShape = vector<pair<int, int>>;

vector<PieceShape> SHAPE[7];

int bgGrid[GH][GW]{};

PieceShape cur;

int cx = 0;
int cy = 0;
int ct = 0;

int sel = 0;
int tick = 0;
int bestScore = 0;

bool active = false;

Px scr[ROWS][COLS];

mt19937 rng{random_device{}()};

// ============================================================================
// BUILD TETROMINO SHAPES
// Chỉ dùng cho hiệu ứng block trang trí ở Menu.
// Không liên quan đến gameplay thật.
// ============================================================================

void buildShapes() {
    static bool done = false;

    if (done) return;
    done = true;

    static const int base[7][8] = {
        {0,0,1,0,2,0,3,0},
        {0,0,1,0,0,1,1,1},
        {0,0,1,0,2,0,1,1},
        {1,0,2,0,0,1,1,1},
        {0,0,1,0,1,1,2,1},
        {0,0,0,1,1,1,2,1},
        {2,0,0,1,1,1,2,1}
    };

    for (int t = 0; t < 7; ++t) {
        PieceShape piece;

        for (int i = 0; i < 4; ++i) {
            piece.push_back({
                base[t][i * 2],
                base[t][i * 2 + 1]
            });
        }

        for (int rotation = 0; rotation < 4; ++rotation) {
            SHAPE[t].push_back(piece);

            for (auto& cell : piece) {
                cell = {-cell.second, cell.first};
            }

            int minX = INT_MAX;
            int minY = INT_MAX;

            for (const auto& cell : piece) {
                minX = min(minX, cell.first);
                minY = min(minY, cell.second);
            }

            for (auto& cell : piece) {
                cell.first -= minX;
                cell.second -= minY;
            }
        }
    }
}

// ============================================================================
// COLLISION CHECK FOR DECORATIVE BLOCKS
// ============================================================================

bool fits(const PieceShape& piece, int ox, int oy) {
    for (const auto& cell : piece) {
        const int x = ox + cell.first;
        const int y = oy + cell.second;

        if (x < 0 || x >= GW || y >= GH) {
            return false;
        }

        if (y >= 0 && bgGrid[y][x]) {
            return false;
        }
    }

    return true;
}

bool rowFull(int row) {
    for (int column = 0; column < GW; ++column) {
        if (!bgGrid[row][column]) {
            return false;
        }
    }

    return true;
}

// ============================================================================
// SPAWN DECORATIVE BLOCK
// ============================================================================

void spawn() {
    const int type = static_cast<int>(rng() % 7);

    int bestRotation = -1;
    int bestX = 0;
    int bestValue = INT_MIN;

    for (int rotation = 0; rotation < 4; ++rotation) {
        const PieceShape& piece = SHAPE[type][rotation];

        for (int x = 0; x < GW; ++x) {
            if (!fits(piece, x, 0)) {
                continue;
            }

            int y = 0;

            while (fits(piece, x, y + 1)) {
                ++y;
            }

            for (const auto& cell : piece) {
                bgGrid[y + cell.second][x + cell.first] = 9;
            }

            int maxY = 0;
            int holes = 0;
            int lines = 0;

            for (const auto& cell : piece) {
                const int yy = y + cell.second;

                maxY = max(maxY, yy);

                if (yy + 1 < GH &&
                    !bgGrid[yy + 1][x + cell.first]) {
                    ++holes;
                }
            }

            for (int row = y;
                 row < GH && row < y + 4;
                 ++row) {

                if (rowFull(row)) {
                    ++lines;
                }
            }

            for (const auto& cell : piece) {
                bgGrid[y + cell.second][x + cell.first] = 0;
            }

            const int value =
                maxY * 2 + lines * 20 - holes * 6;

            if (value > bestValue) {
                bestValue = value;
                bestRotation = rotation;
                bestX = x;
            }
        }
    }

    if (bestRotation < 0) {
        memset(bgGrid, 0, sizeof(bgGrid));
        return;
    }

    cur = SHAPE[type][bestRotation];

    cx = bestX;
    cy = 0;
    ct = type;

    active = true;
}

// ============================================================================
// UPDATE DECORATIVE BLOCK ANIMATION
// ============================================================================

void step() {
    if (!active) {
        spawn();
        return;
    }

    if (fits(cur, cx, cy + 1)) {
        ++cy;
        return;
    }

    for (const auto& cell : cur) {
        bgGrid[cy + cell.second][cx + cell.first] = ct + 1;
    }

    active = false;

    for (int row = GH - 1; row >= 0; --row) {
        if (!rowFull(row)) {
            continue;
        }

        for (int copyRow = row; copyRow > 0; --copyRow) {
            memcpy(
                bgGrid[copyRow],
                bgGrid[copyRow - 1],
                sizeof(bgGrid[copyRow])
            );
        }

        memset(bgGrid[0], 0, sizeof(bgGrid[0]));

        ++row;
    }

    for (int column = 0; column < GW; ++column) {
        if (bgGrid[5][column]) {
            memset(bgGrid, 0, sizeof(bgGrid));
            break;
        }
    }
}

// ============================================================================
// DRAWING FUNCTIONS
// ============================================================================

void put(
    int row,
    int column,
    const string& value,
    int foreground,
    int background
) {
    for (size_t i = 0; i < value.size(); ++column) {
        const unsigned char ch =
            static_cast<unsigned char>(value[i]);

        const size_t length =
            ch >= 0xF0 ? 4 :
            ch >= 0xE0 ? 3 :
            ch >= 0xC0 ? 2 : 1;

        if (row >= 0 && row < GH &&
            column >= 0 && column < SW) {

            scr[row + 1][column + 1] = {
                value.substr(i, length),
                foreground,
                background
            };
        }

        i += length;
    }
}

void fillRect(
    int row1,
    int row2,
    int col1,
    int col2,
    int background
) {
    for (int row = row1; row <= row2; ++row) {
        put(
            row,
            col1,
            string(
                static_cast<size_t>(max(0, col2 - col1 + 1)),
                ' '
            ),
            WHITE,
            background
        );
    }
}

void raw(
    int row,
    int column,
    const char* value,
    int foreground
) {
    if (row >= 0 && row < ROWS &&
        column >= 0 && column < COLS) {

        scr[row][column] = {
            value,
            foreground,
            NAVY
        };
    }
}

// ============================================================================
// COMPOSE MENU SCREEN
// ============================================================================

void compose() {
    for (auto& row : scr) {
        for (auto& pixel : row) {
            pixel = Px{" ", WHITE, NAVY};
        }
    }

    // Outer frame.
    for (int column = 1; column < COLS - 1; ++column) {
        raw(0, column, "─", FRAME);
        raw(ROWS - 1, column, "─", FRAME);
    }

    for (int row = 1; row < ROWS - 1; ++row) {
        raw(row, 0, "│", FRAME);
        raw(row, COLS - 1, "│", FRAME);
    }

    raw(0, 0, "╭", FRAME);
    raw(0, COLS - 1, "╮", FRAME);
    raw(ROWS - 1, 0, "╰", FRAME);
    raw(ROWS - 1, COLS - 1, "╯", FRAME);

    // Decorative background.
    for (int row = 0; row < GH; ++row) {
        for (int column = 0; column < GW; ++column) {
            put(row, 2 * column, "·", DOT, BOARD);
            put(row, 2 * column + 1, " ", DOT, BOARD);

            const int value = bgGrid[row][column];

            if (value >= 1 && value <= 7) {
                put(
                    row,
                    2 * column,
                    "■",
                    PC[value - 1],
                    BOARD
                );
            }
        }
    }

    // Ghost and active decorative block.
    if (active) {
        int ghostY = cy;

        while (fits(cur, cx, ghostY + 1)) {
            ++ghostY;
        }

        if (ghostY > cy) {
            for (const auto& cell : cur) {
                put(
                    ghostY + cell.second,
                    2 * (cx + cell.first),
                    "□",
                    GHOSTC,
                    BOARD
                );
            }
        }

        for (const auto& cell : cur) {
            if (cy + cell.second >= 0) {
                put(
                    cy + cell.second,
                    2 * (cx + cell.first),
                    "■",
                    PC[ct],
                    BOARD
                );
            }
        }
    }

    // Original TETRIS logo.
    static const char* LOGO[6] = {
        "████████╗███████╗████████╗██████╗ ██╗███████╗",
        "╚══██╔══╝██╔════╝╚══██╔══╝██╔══██╗██║██╔════╝",
        "   ██║   █████╗     ██║   ██████╔╝██║███████╗",
        "   ██║   ██╔══╝     ██║   ██╔══██╗██║╚════██║",
        "   ██║   ███████╗   ██║   ██║  ██║██║███████║",
        "   ╚═╝   ╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝╚══════╝"
    };

    fillRect(1, 8, 0, SW - 1, BOARD);

    for (int i = 0; i < 6; ++i) {
        put(
            2 + i,
            1,
            LOGO[i],
            PC[(tick / 5 + i) % 7],
            BOARD
        );
    }

    // Original Menu box.
    fillRect(10, 16, 12, 35, NAVY);

    string borderLine;

    for (int i = 0; i < 22; ++i) {
        borderLine += "─";
    }

    put(
        10,
        12,
        "╭" + borderLine + "╮",
        BOXB,
        NAVY
    );

    put(
        16,
        12,
        "╰" + borderLine + "╯",
        BOXB,
        NAVY
    );

    for (int row = 11; row <= 15; ++row) {
        put(row, 12, "│", BOXB, NAVY);
        put(row, 35, "│", BOXB, NAVY);
    }

    const char* names[3] = {
        "Play",
        "High Score",
        "Exit"
    };

    const int accent[3] = {
        0x5cf05c,
        0xf5e65a,
        0xff7070
    };

    static constexpr int blinkOffsets[4] = {
        0, 1, 2, 1
    };

    const int blink =
        blinkOffsets[(tick / 3) % 4];

    for (int i = 0; i < 3; ++i) {
        const int row = 12 + i;

        const int length =
            static_cast<int>(strlen(names[i]));

        const int column = 24 - length / 2;

        if (i == sel) {
            fillRect(row, row, 14, 33, BOXF);

            put(
                row,
                column,
                names[i],
                accent[i],
                BOXF
            );

            put(
                row,
                15 + blink,
                "▶",
                accent[i],
                BOXF
            );

            put(
                row,
                32 - blink,
                "◀",
                accent[i],
                BOXF
            );
        } else {
            put(
                row,
                column,
                names[i],
                DIMT,
                NAVY
            );
        }
    }

    // High score display.
    const string best =
        " Best score: " + to_string(bestScore) + " ";

    put(
        18,
        (SW - static_cast<int>(best.size())) / 2,
        best,
        0xf5e65a,
        BOARD
    );

    const string help1 =
        " W/S: Move   Enter: Select ";

    const string help2 =
        " In game: P = pause ";

    put(
        20,
        (SW - static_cast<int>(help1.size())) / 2,
        help1,
        0x9aa6e0,
        BOARD
    );

    put(
        21,
        (SW - static_cast<int>(help2.size())) / 2,
        help2,
        0x9aa6e0,
        BOARD
    );
}

// ============================================================================
// RENDER MENU
// ============================================================================

string render() {
    string output = "\x1b[H";

    char buffer[64];

    for (int row = 0; row < ROWS; ++row) {
        int lastForeground = -1;
        int lastBackground = -1;

        for (int column = 0; column < COLS; ++column) {
            const Px& pixel = scr[row][column];

            if (pixel.fg != lastForeground ||
                pixel.bg != lastBackground) {

                snprintf(
                    buffer,
                    sizeof(buffer),
                    "\x1b[38;2;%d;%d;%dm"
                    "\x1b[48;2;%d;%d;%dm",

                    pixel.fg >> 16,
                    (pixel.fg >> 8) & 255,
                    pixel.fg & 255,

                    pixel.bg >> 16,
                    (pixel.bg >> 8) & 255,
                    pixel.bg & 255
                );

                output += buffer;

                lastForeground = pixel.fg;
                lastBackground = pixel.bg;
            }

            output += pixel.ch;
        }

        output += "\x1b[0m\n";
    }

    return output;
}

} // namespace

// ============================================================================
// PUBLIC MENU FUNCTION
// ============================================================================

int showMainMenu(int best) {
    MenuTerminal terminal;

    buildShapes();

    bestScore = max(0, best);

    sel = 0;
    tick = 0;
    active = false;

    memset(bgGrid, 0, sizeof(bgGrid));

    fputs("\x1b[2J\x1b[?25l", stdout);

    int result = 0;

    while (result == 0) {
        int key = -1;

        // Read all pending keys.
        while ((key = readMenuKey()) != -1) {
            if (key == 'w' || key == 'W' ||
                key == KEY_UP) {

                sel = (sel + 2) % 3;

            } else if (key == 's' || key == 'S' ||
                       key == KEY_DOWN) {

                sel = (sel + 1) % 3;

            } else if (key == '\r' ||
                       key == '\n' ||
                       key == ' ') {

                result = sel + 1;

            } else if (key >= '1' && key <= '3') {
                sel = key - '1';
                result = key - '0';
            }
        }

        // Update decorative background animation.
        step();

        // Compose and render current frame.
        compose();

        const string frame = render();

        fwrite(
            frame.data(),
            1,
            frame.size(),
            stdout
        );

        fflush(stdout);

        ++tick;

        this_thread::sleep_for(
            chrono::milliseconds(80)
        );
    }

    fputs("\x1b[0m\x1b[2J\x1b[H", stdout);

    return result;
}
