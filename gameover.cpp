// ============================================================================
// Tetris Menu + Game Over UI
// ---------------------------------------------------------------------------
// Pham vi file:
//   - Chi phu trach giao dien Menu va Game Over.
//   - Khong chua gameplay: khong tao/xoay/di chuyen khoi, khong tinh diem,
//     khong xoa hang va khong quan ly vong doi game.
//   - Cac lua chon cua nguoi dung duoc tra ve duoi dang UiEvent de game engine
//     ben ngoai xu ly.
//   - Game Over: YES -> RetrySelected (game engine quay lai Gameplay).
//                 NO  -> ReturnToMenu (UI chuyen ve MainMenu).
//   - File nay khong tu restart gameplay va khong chua code gameplay.
//
// Tich hop trong game loop:
//   switch (ui.handleKey(key)) {
//       case TetrisUI::UiEvent::RetrySelected:
//           gameState = GameState::Playing; // engine tu reset va chay gameplay
//           break;
//       case TetrisUI::UiEvent::ReturnToMenu:
//           gameState = GameState::Menu;
//           break;
//       default: break;
//   }
//
// Chay preview doc lap:
//   g++ -std=c++17 -O2 tetris_menu_ui.cpp -o tetris_menu_ui
//   ./tetris_menu_ui
//
// Dieu khien preview:
//   Menu      : W/S hoac Up/Down de chon; Enter de phat event.
//   Game Over : A/D hoac Left/Right de chon Yes/No; Enter de phat event.
//   G         : Mo man Game Over de xem giao dien.
//   M         : Quay lai Menu.
//   Q         : Dong chuong trinh preview.
// ============================================================================

#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
    #define NOMINMAX
    #include <windows.h>
    #include <conio.h>
#else
    #include <sys/select.h>
    #include <termios.h>
    #include <unistd.h>
#endif

using namespace std;

namespace TetrisUI {

// ---------------------------------------------------------------------------
// Kich thuoc canvas va bang mau theo Tetris Menu Preview.html.
// ---------------------------------------------------------------------------
constexpr int SCREEN_ROWS = 24;
constexpr int SCREEN_COLS = 50;
constexpr int BOARD_ROWS = 22;
constexpr int BOARD_COLS = 24;

const string COLOR_NAVY  = "#2a2d7a";
const string COLOR_BOARD = "#1b1f2a";
const string COLOR_FRAME = "#4a6aa8";
const string COLOR_DOT   = "#3b4358";
const string COLOR_BOX_BORDER = "#4a7de0";
const string COLOR_BOX_ACTIVE = "#3a53a0";
const string COLOR_TEXT_DIM   = "#aab0e0";
const string COLOR_TEXT_HELP  = "#9aa6e0";
const string COLOR_YELLOW     = "#f5e65a";
const string COLOR_GREEN      = "#5cf05c";
const string COLOR_RED        = "#ff7070";

const array<string, 7> TETRIS_COLORS = {
    "#18c8c8", "#e8e818", "#a010f0", "#2ee82e",
    "#e02020", "#4a5de0", "#e07818"
};

const array<string, 3> MENU_ITEMS = {"Play", "High Score", "Exit"};

// Cac man hinh UI. Game engine ben ngoai co the chuyen man bang setScreen().
enum class ScreenMode {
    MainMenu,
    GameOver
};

// Event UI gui lai cho game engine. File nay chi phat event, khong xu ly gameplay.
enum class UiEvent {
    None,
    PlaySelected,
    HighScoreSelected,
    ExitSelected,
    RetrySelected,
    ReturnToMenu
};

struct Cell {
    string character = " ";
    string foreground = "#ffffff";
    string background = COLOR_NAVY;
};

using Canvas = array<array<Cell, SCREEN_COLS>, SCREEN_ROWS>;

// Tach chuoi UTF-8 de moi ky tu duoc ve vao mot cot cua canvas.
// Cac ky tu khung vien/logo trong preview deu la ky tu UTF-8.
vector<string> splitUtf8(const string& value) {
    vector<string> result;

    for (size_t i = 0; i < value.size();) {
        const unsigned char firstByte = static_cast<unsigned char>(value[i]);
        size_t charLength = 1;

        if ((firstByte & 0xE0) == 0xC0) charLength = 2;
        else if ((firstByte & 0xF0) == 0xE0) charLength = 3;
        else if ((firstByte & 0xF8) == 0xF0) charLength = 4;

        charLength = min(charLength, value.size() - i);
        result.push_back(value.substr(i, charLength));
        i += charLength;
    }

    return result;
}

int textLength(const string& value) {
    return static_cast<int>(splitUtf8(value).size());
}

string repeatCharacter(const string& character, int count) {
    string result;
    for (int i = 0; i < count; ++i) result += character;
    return result;
}

// ---------------------------------------------------------------------------
// Lop UI co the duoc tich hop vao game engine.
// ---------------------------------------------------------------------------
class TetrisMenuUI {
public:
    TetrisMenuUI() {
        clearCanvas();
    }

    // Dat man hinh can hien thi tu game engine.
    void setScreen(ScreenMode newScreen) {
        currentScreen = newScreen;

        // Moi lan mo Game Over deu mac dinh chon Yes; ve Menu thi chon Play.
        if (newScreen == ScreenMode::GameOver) selectedRetryOption = 0;
        if (newScreen == ScreenMode::MainMenu) selectedMenuItem = 0;
    }

    // Cap nhat diem chi de hien thi. Viec tinh diem khong nam trong file UI.
    void setScores(int previousScore, int highScore) {
        lastScore = max(0, previousScore);
        bestScore = max(0, highScore);
    }

    ScreenMode getScreen() const {
        return currentScreen;
    }

    // Nhan phim va tra ve event. Game engine tu quyet dinh hanh dong tiep theo.
    UiEvent handleKey(int key) {
        if (currentScreen == ScreenMode::GameOver) {
            return handleGameOverKey(key);
        }
        return handleMenuKey(key);
    }

    // Ve UI tai frame hien tai. tick duoc dung cho hieu ung nhap nhay.
    void render(int tick, const string& statusMessage = "") {
        compose(tick);
        drawCanvas();
        drawFooter(statusMessage);
        cout << flush;
    }

private:
    Canvas canvas{};
    ScreenMode currentScreen = ScreenMode::MainMenu;
    int selectedMenuItem = 0;
    int selectedRetryOption = 0; // 0 = Yes, 1 = No
    int lastScore = 980;         // So diem mau de preview giao dien.
    int bestScore = 1250;        // So diem mau de preview giao dien.

    // Dua toan bo canvas ve mau nen mac dinh.
    void clearCanvas() {
        for (auto& row : canvas) {
            for (auto& cell : row) {
                cell = Cell{" ", "#ffffff", COLOR_NAVY};
            }
        }
    }

    // Ghi text vao canvas. row/column la toa do ben trong vung noi dung.
    void put(int row, int column, const string& value,
             const string& foreground, const string& background) {
        const vector<string> characters = splitUtf8(value);

        for (size_t i = 0; i < characters.size(); ++i) {
            const int targetColumn = column + static_cast<int>(i);

            if (row >= 0 && row < BOARD_ROWS &&
                targetColumn >= 0 && targetColumn < SCREEN_COLS - 2) {
                canvas[row + 1][targetColumn + 1] =
                    Cell{characters[i], foreground, background};
            }
        }
    }

    // To mau mot hinh chu nhat tren canvas.
    void fill(int firstRow, int lastRow, int firstColumn, int lastColumn,
              const string& background) {
        for (int row = firstRow; row <= lastRow; ++row) {
            if (row < 0 || row >= BOARD_ROWS) continue;

            const int width = max(0, lastColumn - firstColumn + 1);
            put(row, firstColumn, string(static_cast<size_t>(width), ' '),
                "#ffffff", background);
        }
    }

    // Can giua chuoi theo chieu rong cua khung UI.
    void putCentered(int row, const string& value, const string& foreground,
                     const string& background = COLOR_NAVY) {
        const int column = (SCREEN_COLS - textLength(value)) / 2;
        put(row, column, value, foreground, background);
    }

    // Ve vien ngoai va nen cham trang tri. Day chi la background tinh, khong
    // phai board game va khong co logic khoi Tetris.
    void drawFrameAndBackground() {
        // Nen cham trang tri.
        for (int row = 0; row < BOARD_ROWS; ++row) {
            for (int column = 0; column < BOARD_COLS; ++column) {
                put(row, column * 2, ".", COLOR_DOT, COLOR_BOARD);
                put(row, column * 2 + 1, " ", COLOR_DOT, COLOR_BOARD);
            }
        }

        // Vien ngoai cua man hinh.
        for (int column = 1; column < SCREEN_COLS - 1; ++column) {
            canvas[0][column] = Cell{"─", COLOR_FRAME, COLOR_NAVY};
            canvas[SCREEN_ROWS - 1][column] = Cell{"─", COLOR_FRAME, COLOR_NAVY};
        }

        for (int row = 1; row < SCREEN_ROWS - 1; ++row) {
            canvas[row][0] = Cell{"│", COLOR_FRAME, COLOR_NAVY};
            canvas[row][SCREEN_COLS - 1] = Cell{"│", COLOR_FRAME, COLOR_NAVY};
        }

        canvas[0][0] = Cell{"╭", COLOR_FRAME, COLOR_NAVY};
        canvas[0][SCREEN_COLS - 1] = Cell{"╮", COLOR_FRAME, COLOR_NAVY};
        canvas[SCREEN_ROWS - 1][0] = Cell{"╰", COLOR_FRAME, COLOR_NAVY};
        canvas[SCREEN_ROWS - 1][SCREEN_COLS - 1] = Cell{"╯", COLOR_FRAME, COLOR_NAVY};
    }

    // Ve logo TETRIS 6 dong, mau sac thay doi theo tick.
    void drawLogo(int tick) {
        const array<string, 6> logo = {
            "████████╗███████╗████████╗██████╗ ██╗███████╗",
            "╚══██╔══╝██╔════╝╚══██╔══╝██╔══██╗██║██╔════╝",
            "   ██║   █████╗     ██║   ██████╔╝██║███████╗",
            "   ██║   ██╔══╝     ██║   ██╔══██╗██║╚════██║",
            "   ██║   ███████╗   ██║   ██║  ██║██║███████║",
            "   ╚═╝   ╚══════╝   ╚═╝   ╚═╝  ╚═╝╚═╝╚══════╝"
        };

        for (int row = 0; row < static_cast<int>(logo.size()); ++row) {
            const int colorIndex = ((tick / 5) + row) %
                                   static_cast<int>(TETRIS_COLORS.size());
            put(2 + row, 1, logo[row], TETRIS_COLORS[colorIndex], COLOR_BOARD);
        }
    }

    // Ve khung Menu chinh va cac lua chon Play / High Score / Exit.
    void drawMainMenu(int tick) {
        fill(1, 8, 0, SCREEN_COLS - 3, COLOR_BOARD);
        drawLogo(tick);

        // Khung lua chon menu.
        fill(10, 16, 12, 35, COLOR_NAVY);
        put(10, 12, "╭" + repeatCharacter("─", 22) + "╮",
            COLOR_BOX_BORDER, COLOR_NAVY);
        put(16, 12, "╰" + repeatCharacter("─", 22) + "╯",
            COLOR_BOX_BORDER, COLOR_NAVY);

        for (int row = 11; row <= 15; ++row) {
            put(row, 12, "│", COLOR_BOX_BORDER, COLOR_NAVY);
            put(row, 35, "│", COLOR_BOX_BORDER, COLOR_NAVY);
        }

        // Mui ten hai ben nhap nhay nhe o dong dang duoc chon.
        const array<int, 4> arrowOffsets = {0, 1, 2, 1};
        const int arrowOffset = arrowOffsets[(tick / 3) % arrowOffsets.size()];

        for (int i = 0; i < static_cast<int>(MENU_ITEMS.size()); ++i) {
            const int row = 12 + i;
            const int column = 24 - (textLength(MENU_ITEMS[i]) / 2);

            if (i == selectedMenuItem) {
                fill(row, row, 14, 33, COLOR_BOX_ACTIVE);
                put(row, column, MENU_ITEMS[i], TETRIS_COLORS[i == 0 ? 3 : i == 1 ? 1 : 4],
                    COLOR_BOX_ACTIVE);
                put(row, 15 + arrowOffset, "▶", TETRIS_COLORS[i == 0 ? 3 : i == 1 ? 1 : 4],
                    COLOR_BOX_ACTIVE);
                put(row, 32 - arrowOffset, "◀", TETRIS_COLORS[i == 0 ? 3 : i == 1 ? 1 : 4],
                    COLOR_BOX_ACTIVE);
            } else {
                put(row, column, MENU_ITEMS[i], COLOR_TEXT_DIM, COLOR_NAVY);
            }
        }

        // Diem cao chi de hien thi; gia tri duoc cap nhat bang setScores().
        const string scoreLabel = " Best score: " + to_string(bestScore) + " ";
        const string moveHelp = " W/S: Move   Enter: Select ";
        const string gameOverHelp = " G: Preview Game Over ";

        putCentered(18, scoreLabel, COLOR_YELLOW, COLOR_BOARD);
        putCentered(20, moveHelp, COLOR_TEXT_HELP, COLOR_BOARD);
        putCentered(21, gameOverHelp, COLOR_TEXT_HELP, COLOR_BOARD);
    }

    // Ve man Game Over va tuy chon Retry Yes/No.
    void drawGameOver(int tick) {
        fill(5, 16, 10, 37, COLOR_NAVY);
        put(5, 10, "╭" + repeatCharacter("─", 26) + "╮",
            COLOR_BOX_BORDER, COLOR_NAVY);
        put(16, 10, "╰" + repeatCharacter("─", 26) + "╯",
            COLOR_BOX_BORDER, COLOR_NAVY);

        for (int row = 6; row <= 15; ++row) {
            put(row, 10, "│", COLOR_BOX_BORDER, COLOR_NAVY);
            put(row, 37, "│", COLOR_BOX_BORDER, COLOR_NAVY);
        }

        const bool blink = ((tick / 4) % 2) != 0;
        putCentered(7, "G A M E   O V E R", blink ? COLOR_RED : "#e02020");
        putCentered(9, "Previous score: " + to_string(lastScore), COLOR_YELLOW);

        if (lastScore > bestScore) {
            if (blink) putCentered(10, "* NEW BEST! *", COLOR_GREEN);
        } else {
            putCentered(10, "Best score: " + to_string(bestScore), COLOR_TEXT_DIM);
        }

        putCentered(12, "Retry?", "#ffffff");

        const array<string, 2> selectedLabels = {"▶ Yes ◀", "▶ No ◀"};
        const array<string, 2> idleLabels = {"  Yes  ", "  No  "};
        const array<int, 2> columns = {14, 26};
        const array<int, 2> widths = {9, 8};
        const array<string, 2> activeColors = {COLOR_GREEN, COLOR_RED};

        for (int i = 0; i < 2; ++i) {
            if (i == selectedRetryOption) {
                fill(14, 14, columns[i], columns[i] + widths[i] - 1,
                     COLOR_BOX_ACTIVE);
                put(14, columns[i] + 1, selectedLabels[i], activeColors[i],
                    COLOR_BOX_ACTIVE);
            } else {
                put(14, columns[i] + 1, idleLabels[i], COLOR_TEXT_DIM,
                    COLOR_NAVY);
            }
        }

        const string selectHelp = " A/D: Move   Enter: Select ";
        const string shortcutHelp = " Y = Yes   N = No ";
        putCentered(20, selectHelp, COLOR_TEXT_HELP, COLOR_BOARD);
        putCentered(21, shortcutHelp, COLOR_TEXT_HELP, COLOR_BOARD);
    }

    // Ghep cac thanh phan UI vao canvas theo man hinh hien tai.
    void compose(int tick) {
        clearCanvas();
        drawFrameAndBackground();

        if (currentScreen == ScreenMode::GameOver) {
            drawGameOver(tick);
        } else {
            drawMainMenu(tick);
        }
    }

    // Chuyen ma mau HEX thanh tham so RGB cho ANSI true-color.
    string toRgb(const string& hexColor) const {
        if (hexColor.size() != 7 || hexColor[0] != '#') {
            return "255;255;255";
        }

        const int red = stoi(hexColor.substr(1, 2), nullptr, 16);
        const int green = stoi(hexColor.substr(3, 2), nullptr, 16);
        const int blue = stoi(hexColor.substr(5, 2), nullptr, 16);

        return to_string(red) + ";" + to_string(green) + ";" + to_string(blue);
    }

    // In canvas ra terminal, chi doi ANSI color khi mau foreground/background doi.
    void drawCanvas() const {
        cout << "\x1b[H";
        string previousForeground;
        string previousBackground;
        bool firstCell = true;

        for (int row = 0; row < SCREEN_ROWS; ++row) {
            for (int column = 0; column < SCREEN_COLS; ++column) {
                const Cell& cell = canvas[row][column];

                if (firstCell || cell.foreground != previousForeground ||
                    cell.background != previousBackground) {
                    cout << "\x1b[38;2;" << toRgb(cell.foreground)
                         << ";48;2;" << toRgb(cell.background) << 'm';
                    previousForeground = cell.foreground;
                    previousBackground = cell.background;
                    firstCell = false;
                }

                cout << cell.character;
            }

            if (row != SCREEN_ROWS - 1) cout << "\r\n";
        }

        cout << "\x1b[0m\r\n";
    }

    // Dong trang thai nam ben duoi UI; trong game that co the bo qua/thay the.
    void drawFooter(const string& statusMessage) const {
        string message = statusMessage;

        if (message.empty()) {
            if (currentScreen == ScreenMode::GameOver) {
                message = "Game Over: A/D or Left/Right, Enter, Y = Yes, N = No, M = Menu, Q = Quit preview.";
            } else {
                message = "Menu: W/S or Up/Down, Enter = Select, G = Game Over preview, Q = Quit preview.";
            }
        }

        cout << "\x1b[" << SCREEN_ROWS + 1 << ";1H"
             << "\x1b[38;2;154;166;224m" << message << "\x1b[K";
    }

    // Xu ly phim tai man Menu. Khong thuc hien hanh dong game.
    UiEvent handleMenuKey(int key) {
        if (key == 1001 || key == 'w' || key == 'W') {
            selectedMenuItem = (selectedMenuItem + 2) % 3;
        } else if (key == 1002 || key == 's' || key == 'S') {
            selectedMenuItem = (selectedMenuItem + 1) % 3;
        } else if (key == 13 || key == 10 || key == ' ') {
            switch (selectedMenuItem) {
                case 0: return UiEvent::PlaySelected;
                case 1: return UiEvent::HighScoreSelected;
                case 2: return UiEvent::ExitSelected;
            }
        } else if (key >= '1' && key <= '3') {
            selectedMenuItem = key - '1';
            return handleMenuKey(13);
        } else if (key == 'g' || key == 'G') {
            currentScreen = ScreenMode::GameOver;
            selectedRetryOption = 0;
        }

        return UiEvent::None;
    }

    // Xu ly phim tai Game Over. Yes/No chi phat event de game engine xu ly.
    UiEvent handleGameOverKey(int key) {
        if (key == 1003 || key == 1004 ||
            key == 'a' || key == 'A' || key == 'd' || key == 'D') {
            selectedRetryOption = 1 - selectedRetryOption;
        } else if (key == 13 || key == 10 || key == ' ') {
            if (selectedRetryOption == 0) {
                // YES: chi gui event. Game engine se tu restart va vao Gameplay.
                return UiEvent::RetrySelected;
            }

            // NO: quay ve Menu, khong thoat chuong trinh.
            setScreen(ScreenMode::MainMenu);
            return UiEvent::ReturnToMenu;
        } else if (key == 'y' || key == 'Y') {
            selectedRetryOption = 0;
            return UiEvent::RetrySelected;
        } else if (key == 'n' || key == 'N') {
            selectedRetryOption = 1;
            setScreen(ScreenMode::MainMenu);
            return UiEvent::ReturnToMenu;
        } else if (key == 'm' || key == 'M') {
            // M la phim tat quay ve Menu, tuong duong chon NO.
            setScreen(ScreenMode::MainMenu);
            return UiEvent::ReturnToMenu;
        }

        return UiEvent::None;
    }
};

} // namespace TetrisUI

// ============================================================================
// Phan preview terminal doc lap.
// Phan nay chi giup xem va tuong tac voi UI; khong co gameplay.
// Khi tich hop vao project, game engine co the su dung TetrisMenuUI truc tiep
// va bo phan main()/TerminalGuard/pollKey() neu project da co input system.
// ============================================================================

namespace PreviewTerminal {

#ifdef _WIN32
class TerminalGuard {
public:
    TerminalGuard() {
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);

        HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
        if (output != INVALID_HANDLE_VALUE && GetConsoleMode(output, &oldMode)) {
            SetConsoleMode(output, oldMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
            changedMode = true;
        }

        cout << "\x1b[?25l\x1b[?7l" << flush;
    }

    ~TerminalGuard() {
        cout << "\x1b[0m\x1b[?7h\x1b[?25h\n" << flush;
        if (changedMode) {
            SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), oldMode);
        }
    }

private:
    DWORD oldMode = 0;
    bool changedMode = false;
};

int pollKey() {
    if (!_kbhit()) return -1;

    const int key = _getch();
    if (key == 0 || key == 224) {
        const int extendedKey = _getch();
        switch (extendedKey) {
            case 72: return 1001; // Up
            case 80: return 1002; // Down
            case 75: return 1003; // Left
            case 77: return 1004; // Right
            default: return -1;
        }
    }

    return key;
}
#else
class TerminalGuard {
public:
    TerminalGuard() {
        if (isatty(STDIN_FILENO) && tcgetattr(STDIN_FILENO, &oldSettings) == 0) {
            termios rawSettings = oldSettings;
            rawSettings.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
            rawSettings.c_iflag &= static_cast<tcflag_t>(~(IXON | ICRNL));
            rawSettings.c_oflag &= static_cast<tcflag_t>(~OPOST);
            rawSettings.c_cc[VMIN] = 0;
            rawSettings.c_cc[VTIME] = 0;

            tcsetattr(STDIN_FILENO, TCSANOW, &rawSettings);
            active = true;
        }

        cout << "\x1b[?25l\x1b[?7l" << flush;
    }

    ~TerminalGuard() {
        cout << "\x1b[0m\x1b[?7h\x1b[?25h\n" << flush;
        if (active) tcsetattr(STDIN_FILENO, TCSANOW, &oldSettings);
    }

private:
    termios oldSettings{};
    bool active = false;
};

bool readByte(char& output, int timeoutMicroseconds) {
    fd_set inputSet;
    FD_ZERO(&inputSet);
    FD_SET(STDIN_FILENO, &inputSet);

    timeval timeout{};
    timeout.tv_sec = 0;
    timeout.tv_usec = timeoutMicroseconds;

    const int ready = select(STDIN_FILENO + 1, &inputSet, nullptr, nullptr, &timeout);
    if (ready <= 0) return false;

    return read(STDIN_FILENO, &output, 1) == 1;
}

int pollKey() {
    char key = 0;
    if (!readByte(key, 0)) return -1;

    if (key == '\x1b') {
        char bracket = 0;
        char direction = 0;

        if (readByte(bracket, 3000) && bracket == '[' &&
            readByte(direction, 3000)) {
            switch (direction) {
                case 'A': return 1001; // Up
                case 'B': return 1002; // Down
                case 'D': return 1003; // Left
                case 'C': return 1004; // Right
            }
        }

        return -1;
    }

    return static_cast<unsigned char>(key);
}
#endif

} // namespace PreviewTerminal

// Chuyen UiEvent thanh dong thong bao de nguoi xem biet UI da gui event nao.
// Trong game that, thay phan nay bang viec goi ham cua game engine.
string eventToMessage(TetrisUI::UiEvent event) {
    using TetrisUI::UiEvent;

    switch (event) {
        case UiEvent::PlaySelected:
            return "UI EVENT: PlaySelected - game engine xu ly viec bat dau game.";
        case UiEvent::HighScoreSelected:
            return "UI EVENT: HighScoreSelected - game engine mo man High Score.";
        case UiEvent::ExitSelected:
            return "UI EVENT: ExitSelected - game engine xu ly viec thoat.";
        case UiEvent::RetrySelected:
            return "UI EVENT: RetrySelected - game engine chuyen lai Gameplay va restart vong choi.";
        case UiEvent::ReturnToMenu:
            return "UI EVENT: ReturnToMenu - quay ve Main Menu.";
        case UiEvent::None:
        default:
            return "";
    }
}

#ifndef NO_DEMO_MAIN
int main() {
    using namespace chrono;
    using TetrisUI::TetrisMenuUI;
    using TetrisUI::UiEvent;

    try {
        PreviewTerminal::TerminalGuard terminal;
        TetrisMenuUI ui;

        // Diem mau de xem layout. Game that se truyen diem bang ui.setScores().
        ui.setScores(980, 1250);

        cout << "\x1b[2J";
        bool closePreview = false;
        int tick = 0;
        string statusMessage;
        auto nextFrame = steady_clock::now();

        while (!closePreview) {
            const int key = PreviewTerminal::pollKey();

            if (key == 'q' || key == 'Q') {
                closePreview = true;
                continue;
            }

            if (key != -1) {
                const UiEvent event = ui.handleKey(key);
                if (event != UiEvent::None) {
                    statusMessage = eventToMessage(event);
                }
            }

            const auto now = steady_clock::now();
            if (now >= nextFrame) {
                ui.render(tick++, statusMessage);
                nextFrame = now + milliseconds(80);
            }

            this_thread::sleep_for(milliseconds(2));
        }
    } catch (const exception& error) {
        cerr << "Preview error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
#endif // NO_DEMO_MAIN
