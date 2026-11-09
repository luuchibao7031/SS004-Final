// tetris_menu.cpp - Menu Tetris (ANSI truecolor, console)
// Build: g++ -std=c++17 tetris_menu.cpp -o tetris_menu
// Windows 10+ (Windows Terminal / cmd moi) va Linux/macOS deu chay duoc.
// Ghep vao project: bo main() (hoac define NO_DEMO_MAIN), goi showMainMenu() -> 1=Play 2=High Score 3=Exit
#include <algorithm>
#include <chrono>
#include <climits>
#include <cstdio>
#include <cstring>
#include <random>
#include <string>
#include <thread>
#include <vector>
#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif
using namespace std;
 
namespace {
constexpr int GW = 24, GH = 22, SW = GW * 2, ROWS = GH + 2, COLS = SW + 2;
// Mau lay theo anh: navy + board toi + khung xanh
constexpr int NAVY = 0x2a2d7a, BOARD = 0x1b1f2a, FRAME = 0x4a6aa8, DOT = 0x3b4358;
constexpr int GHOSTC = 0x6f8fd0, BOXB = 0x4a7de0, BOXF = 0x3a53a0, DIMT = 0xaab0e0, WHITE = 0xffffff;
constexpr int PC[7] = {0x18c8c8, 0xe8e818, 0xa010f0, 0x2ee82e, 0xe02020, 0x4a5de0, 0xe07818};
 
struct Px { string ch = " "; int fg = WHITE, bg = NAVY; };
using Piece = vector<pair<int, int>>;
 
vector<Piece> SHAPE[7];
int bgGrid[GH][GW];
Piece cur;
int cx = 0, cy = 0, ct = 0, sel = 0, tick = 0, bestScore = 0;
bool active = false;
Px scr[ROWS][COLS];
mt19937 rng{random_device{}()};
 
void buildShapes() {
    static bool done = false;
    if (done) return;
    done = true;
    static const int base[7][8] = {{0,0,1,0,2,0,3,0},{0,0,1,0,0,1,1,1},{0,0,1,0,2,0,1,1},{1,0,2,0,0,1,1,1},
                                   {0,0,1,0,1,1,2,1},{0,0,0,1,1,1,2,1},{2,0,0,1,1,1,2,1}};
    for (int t = 0; t < 7; t++) {
        Piece p;
        for (int i = 0; i < 4; i++) p.push_back({base[t][i * 2], base[t][i * 2 + 1]});
        for (int r = 0; r < 4; r++) {
            SHAPE[t].push_back(p);
            for (auto& q : p) q = {-q.second, q.first};
            int mx = INT_MAX, my = INT_MAX;
            for (auto& q : p) { mx = min(mx, q.first); my = min(my, q.second); }
            for (auto& q : p) { q.first -= mx; q.second -= my; }
        }
    }
}
 
bool fits(const Piece& p, int ox, int oy) {
    for (auto& q : p) {
        int x = ox + q.first, y = oy + q.second;
        if (x < 0 || x >= GW || y >= GH) return false;
        if (y >= 0 && bgGrid[y][x]) return false;
    }
    return true;
}
bool rowFull(int r) { for (int c = 0; c < GW; c++) if (!bgGrid[r][c]) return false; return true; }
 
// AI tu xep block o nen: chon cho dat tot nhat
void spawn() {
    int t = rng() % 7, bR = -1, bX = 0, best = INT_MIN;
    for (int r = 0; r < 4; r++) {
        const Piece& p = SHAPE[t][r];
        for (int x = 0; x < GW; x++) {
            if (!fits(p, x, 0)) continue;
            int y = 0;
            while (fits(p, x, y + 1)) y++;
            for (auto& q : p) bgGrid[y + q.second][x + q.first] = 9;
            int maxY = 0, holes = 0, lines = 0;
            for (auto& q : p) {
                int yy = y + q.second;
                maxY = max(maxY, yy);
                if (yy + 1 < GH && !bgGrid[yy + 1][x + q.first]) holes++;
            }
            for (int rr = y; rr < GH && rr < y + 4; rr++) if (rowFull(rr)) lines++;
            for (auto& q : p) bgGrid[y + q.second][x + q.first] = 0;
            int s = maxY * 2 + lines * 20 - holes * 6;
            if (s > best) { best = s; bR = r; bX = x; }
        }
    }
    if (bR < 0) { memset(bgGrid, 0, sizeof bgGrid); return; }
    cur = SHAPE[t][bR]; cx = bX; cy = 0; ct = t; active = true;
}
 
void step() {
    if (!active) { spawn(); return; }
    if (fits(cur, cx, cy + 1)) { cy++; return; }
    for (auto& q : cur) bgGrid[cy + q.second][cx + q.first] = ct + 1;
    active = false;
    for (int r = GH - 1; r >= 0; r--)
        if (rowFull(r)) {
            for (int k = r; k > 0; k--) memcpy(bgGrid[k], bgGrid[k - 1], sizeof bgGrid[k]);
            memset(bgGrid[0], 0, sizeof bgGrid[0]);
            r++;
        }
    for (int c = 0; c < GW; c++) if (bgGrid[5][c]) { memset(bgGrid, 0, sizeof bgGrid); break; }
}
 
// toa do (r,c) la toa do ben trong board (0..GH-1, 0..SW-1)
void put(int r, int c, const string& s, int fg, int bg) {
    for (size_t i = 0; i < s.size(); c++) {
        unsigned char ch = s[i];
        size_t n = ch >= 0xF0 ? 4 : ch >= 0xE0 ? 3 : ch >= 0xC0 ? 2 : 1;
        if (r >= 0 && r < GH && c >= 0 && c < SW) scr[r + 1][c + 1] = {s.substr(i, n), fg, bg};
        i += n;
    }
}
void fillRect(int r1, int r2, int c1, int c2, int bg) {
    for (int r = r1; r <= r2; r++) put(r, c1, string(c2 - c1 + 1, ' '), WHITE, bg);
}
void raw(int r, int c, const char* s, int fg) { scr[r][c] = {s, fg, NAVY}; }
 
void compose() {
    for (auto& row : scr) for (auto& p : row) p = Px{" ", WHITE, NAVY};
    for (int c = 1; c < COLS - 1; c++) { raw(0, c, "─", FRAME); raw(ROWS - 1, c, "─", FRAME); }
    for (int r = 1; r < ROWS - 1; r++) { raw(r, 0, "│", FRAME); raw(r, COLS - 1, "│", FRAME); }
    raw(0, 0, "╭", FRAME); raw(0, COLS - 1, "╮", FRAME); raw(ROWS - 1, 0, "╰", FRAME); raw(ROWS - 1, COLS - 1, "╯", FRAME);
 
    for (int r = 0; r < GH; r++)
        for (int c = 0; c < GW; c++) {
            put(r, 2 * c, "·", DOT, BOARD); put(r, 2 * c + 1, " ", DOT, BOARD);
            int v = bgGrid[r][c];
            if (v >= 1 && v <= 7) put(r, 2 * c, "■", PC[v - 1], BOARD);
        }
    if (active) {
        int gy = cy;
        while (fits(cur, cx, gy + 1)) gy++;
        if (gy > cy) for (auto& q : cur) put(gy + q.second, 2 * (cx + q.first), "□", GHOSTC, BOARD);
        for (auto& q : cur) if (cy + q.second >= 0) put(cy + q.second, 2 * (cx + q.first), "■", PC[ct], BOARD);
    }
 
    static const char* LOGO[6] = {
        "████████╗███████╗████████╗██████╗ ██╗███████╗",
        "╚══██╔══╝██╔════╝╚══██╔══╝██╔══██╗██║██╔════╝",
        "   ██║   █████╗     ██║   ██████╔╝██║███████╗",
        "   ██║   ██╔══╝     ██║   ██╔══██╗██║╚════██║",
        "   ██║   ███████╗   ██║   ██║  ██║██║███████║",
        "   ╚═╝   ╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝╚══════╝"};
    fillRect(1, 8, 0, SW - 1, BOARD);
    for (int i = 0; i < 6; i++) put(2 + i, 1, LOGO[i], PC[(tick / 5 + i) % 7], BOARD);
 
    // Menu box (giong o Score/Level trong anh)
    fillRect(10, 16, 12, 35, NAVY);
    string line;
    for (int i = 0; i < 22; i++) line += "─";
    put(10, 12, "╭" + line + "╮", BOXB, NAVY);
    put(16, 12, "╰" + line + "╯", BOXB, NAVY);
    for (int r = 11; r <= 15; r++) { put(r, 12, "│", BOXB, NAVY); put(r, 35, "│", BOXB, NAVY); }
    const char* names[3] = {"Play", "High Score", "Exit"};
    const int acc[3] = {0x5cf05c, 0xf5e65a, 0xff7070};
    int bn = (int[]){0, 1, 2, 1}[(tick / 3) % 4];
    for (int i = 0; i < 3; i++) {
        int row = 12 + i, len = (int)strlen(names[i]), col = 24 - len / 2;
        if (i == sel) {
            fillRect(row, row, 14, 33, BOXF);
            put(row, col, names[i], acc[i], BOXF);
            put(row, 15 + bn, "▶", acc[i], BOXF);
            put(row, 32 - bn, "◀", acc[i], BOXF);
        } else put(row, col, names[i], DIMT, NAVY);
    }
    string bs = " Best score: " + to_string(bestScore) + " ";
    put(18, (SW - (int)bs.size()) / 2, bs, 0xf5e65a, BOARD);
    string h1 = " W/S: Move   Enter: Select ", h2 = " In game: P = pause ";
    put(20, (SW - (int)h1.size()) / 2, h1, 0x9aa6e0, BOARD);
    put(21, (SW - (int)h2.size()) / 2, h2, 0x9aa6e0, BOARD);
}
 
string render() {
    string o = "\x1b[H";
    char b[64];
    for (int r = 0; r < ROWS; r++) {
        int lf = -1, lb = -1;
        for (int c = 0; c < COLS; c++) {
            const Px& p = scr[r][c];
            if (p.fg != lf || p.bg != lb) {
                snprintf(b, sizeof b, "\x1b[38;2;%d;%d;%dm\x1b[48;2;%d;%d;%dm", p.fg >> 16, (p.fg >> 8) & 255, p.fg & 255,
                         p.bg >> 16, (p.bg >> 8) & 255, p.bg & 255);
                o += b; lf = p.fg; lb = p.bg;
            }
            o += p.ch;
        }
        o += "\x1b[0m\n";
    }
    return o;
}
 
#ifdef _WIN32
int readKey() {
    if (!_kbhit()) return -1;
    int c = _getch();
    if (c == 0 || c == 224) { int d = _getch(); return d == 72 ? 'w' : d == 80 ? 's' : -1; }
    return c;
}
struct Term {
    Term() {
        SetConsoleOutputCP(CP_UTF8);
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD m = 0; GetConsoleMode(h, &m); SetConsoleMode(h, m | 0x0004);
    }
};
#else
struct Term {
    termios old;
    Term() { tcgetattr(0, &old); termios t = old; t.c_lflag &= ~(ICANON | ECHO); t.c_cc[VMIN] = 0; t.c_cc[VTIME] = 0; tcsetattr(0, TCSANOW, &t); }
    ~Term() { tcsetattr(0, TCSANOW, &old); }
};
int readKey() {
    unsigned char c;
    if (read(0, &c, 1) != 1) return -1;
    if (c == 27) {
        unsigned char s[2];
        if (read(0, &s[0], 1) == 1 && read(0, &s[1], 1) == 1 && s[0] == '[') return s[1] == 'A' ? 'w' : s[1] == 'B' ? 's' : -1;
        return -1;
    }
    return c;
}
#endif
}  // namespace
 
// Tra ve: 1 = Play, 2 = High Score, 3 = Exit
int showMainMenu(int best = 0) {
    Term term;
    buildShapes();
    bestScore = best; sel = 0; tick = 0; active = false;
    memset(bgGrid, 0, sizeof bgGrid);
    fputs("\x1b[2J\x1b[?25l", stdout);
    int result = 0;
    while (!result) {
        int k;
        while ((k = readKey()) != -1) {
            if (k == 'w' || k == 'W') sel = (sel + 2) % 3;
            else if (k == 's' || k == 'S') sel = (sel + 1) % 3;
            else if (k == '\r' || k == '\n' || k == ' ') result = sel + 1;
            else if (k >= '1' && k <= '3') { sel = k - '1'; result = k - '0'; }
        }
        step(); compose(); tick++;
        string f = render();
        fwrite(f.data(), 1, f.size(), stdout);
        fflush(stdout);
        this_thread::sleep_for(chrono::milliseconds(80));
    }
    fputs("\x1b[0m\x1b[2J\x1b[H\x1b[?25h", stdout);
    return result;
}
 
#ifndef NO_DEMO_MAIN
int main() {
    int c = showMainMenu(1250);
    printf("Ban chon: %d\n", c);
    return 0;
}
#endif