
// ============================================================================
// TV5 - GAME OVER / PAUSE / HIGH SCORE UI
// Project: SS004-Final
//
// Chuc nang:
//   1. Game Over: Y = Retry, N = Return to Menu.
//      A/D hoac Left/Right de chon, Enter de xac nhan.
//   2. Pause: P / Enter / R = Resume, M / ESC = Return to Menu.
//   3. High Score: hien thi diem cao nhat, nhan phim de quay lai Menu.
//
// Luu y:
//   - UI chi tra UiEvent, khong tu restart gameplay.
//   - Khong co main() rieng.
//   - Khong can sua Input.h chi de doc phim trong cac man hinh nay.
// ============================================================================

#include "GameOverUI.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
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

namespace TetrisUI {
namespace {

// ============================================================================
// CONSTANTS
// ============================================================================

constexpr int KEY_LEFT = 1003;
constexpr int KEY_RIGHT = 1004;
constexpr int KEY_ESCAPE = 1005;

constexpr int SCREEN_ROWS = 24;
constexpr int SCREEN_COLS = 50;
constexpr int BOARD_ROWS = 22;
constexpr int BOARD_COLS = 24;

// Original TV5 color palette.
const std::string COLOR_NAVY = "#2a2d7a";
const std::string COLOR_BOARD = "#1b1f2a";
const std::string COLOR_FRAME = "#4a6aa8";
const std::string COLOR_DOT = "#3b4358";
const std::string COLOR_BOX_BORDER = "#4a7de0";
const std::string COLOR_BOX_ACTIVE = "#3a53a0";
const std::string COLOR_TEXT_DIM = "#aab0e0";
const std::string COLOR_TEXT_HELP = "#9aa6e0";
const std::string COLOR_YELLOW = "#f5e65a";
const std::string COLOR_GREEN = "#5cf05c";
const std::string COLOR_RED = "#ff7070";

// ============================================================================
// TERMINAL CONTROL
// Doc phim rieng, khong lam thay doi API Input cua nhom.
// ============================================================================

class GameOverTerminal {
public:
    GameOverTerminal() {
#ifdef _WIN32
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);

        const HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);

        if (output != INVALID_HANDLE_VALUE &&
            GetConsoleMode(output, &oldConsoleMode_)) {

            if (SetConsoleMode(
                    output,
                    oldConsoleMode_ | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
                changedConsoleMode_ = true;
            }
        }
#else
        if (isatty(STDIN_FILENO) &&
            tcgetattr(STDIN_FILENO, &oldTerminalMode_) == 0) {

            termios raw = oldTerminalMode_;

            raw.c_lflag &= static_cast<tcflag_t>(
                ~(ICANON | ECHO)
            );

            raw.c_iflag &= static_cast<tcflag_t>(
                ~(IXON | ICRNL)
            );

            raw.c_oflag &= static_cast<tcflag_t>(
                ~OPOST
            );

            raw.c_cc[VMIN] = 0;
            raw.c_cc[VTIME] = 0;

            terminalModeSaved_ =
                tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0;
        }
#endif

        std::cout << "\x1b[?25l\x1b[?7l" << std::flush;
    }

    ~GameOverTerminal() {
        std::cout << "\x1b[0m\x1b[?7h\x1b[?25h\n" << std::flush;

#ifdef _WIN32
        if (changedConsoleMode_) {
            SetConsoleMode(
                GetStdHandle(STD_OUTPUT_HANDLE),
                oldConsoleMode_
            );
        }
#else
        if (terminalModeSaved_) {
            tcsetattr(
                STDIN_FILENO,
                TCSANOW,
                &oldTerminalMode_
            );
        }
#endif
    }

    GameOverTerminal(const GameOverTerminal&) = delete;
    GameOverTerminal& operator=(const GameOverTerminal&) = delete;

private:

#ifdef _WIN32
    DWORD oldConsoleMode_ = 0;
    bool changedConsoleMode_ = false;
#else
    termios oldTerminalMode_{};
    bool terminalModeSaved_ = false;
#endif
};

// ============================================================================
// NON-BLOCKING KEY READER
// ============================================================================

#ifndef _WIN32

bool readTerminalByte(char& value, long timeoutMicroseconds) {
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

    return ready > 0 && read(STDIN_FILENO, &value, 1) == 1;
}

#endif

int readUiKey() {
#ifdef _WIN32

    if (!_kbhit()) return -1;

    const int key = _getch();

    if (key == 0 || key == 224) {
        switch (_getch()) {
            case 75: return KEY_LEFT;
            case 77: return KEY_RIGHT;
            default: return -1;
        }
    }

    return key;

#else

    char key = 0;

    if (!readTerminalByte(key, 0)) {
        return -1;
    }

    if (static_cast<unsigned char>(key) == 0x1B) {
        char bracket = 0;
        char direction = 0;

        if (readTerminalByte(bracket, 12000) &&
            bracket == '[' &&
            readTerminalByte(direction, 12000)) {

            switch (direction) {
                case 'D': return KEY_LEFT;
                case 'C': return KEY_RIGHT;
                default: return -1;
            }
        }

        return KEY_ESCAPE;
    }

    return static_cast<unsigned char>(key);

#endif
}

// ============================================================================
// CANVAS
// ============================================================================

struct Cell {
    std::string character = " ";
    std::string foreground = "#ffffff";
    std::string background = COLOR_NAVY;
};

using Canvas = std::array<
    std::array<Cell, SCREEN_COLS>,
    SCREEN_ROWS
>;

enum class ScreenMode {
    GameOver,
    Paused,
    HighScore
};

// ============================================================================
// UTF-8 HELPERS
// ============================================================================

std::vector<std::string> splitUtf8(const std::string& value) {
    std::vector<std::string> result;

    for (std::size_t i = 0; i < value.size();) {
        const unsigned char firstByte =
            static_cast<unsigned char>(value[i]);

        std::size_t charLength = 1;

        if ((firstByte & 0xE0) == 0xC0) {
            charLength = 2;
        } else if ((firstByte & 0xF0) == 0xE0) {
            charLength = 3;
        } else if ((firstByte & 0xF8) == 0xF0) {
            charLength = 4;
        }

        charLength = std::min(
            charLength,
            value.size() - i
        );

        result.push_back(value.substr(i, charLength));
        i += charLength;
    }

    return result;
}

int textLength(const std::string& value) {
    return static_cast<int>(splitUtf8(value).size());
}

std::string repeatCharacter(
    const std::string& character,
    int count
) {
    std::string result;

    for (int i = 0; i < count; ++i) {
        result += character;
    }

    return result;
}

// ============================================================================
// GAME OVER UI CLASS
// ============================================================================

class GameOverUI {
public:

    GameOverUI(
        ScreenMode screen,
        int currentScore,
        int highScore
    )
        : screen_(screen),
          lastScore_(std::max(0, currentScore)),
          bestScore_(std::max(0, highScore)) {

        clearCanvas();
    }

    // Xu ly phim va tra event cho Game Engine.
    UiEvent handleKey(int key) {
        switch (screen_) {

            case ScreenMode::GameOver:
                return handleGameOverKey(key);

            case ScreenMode::Paused:
                return handlePauseKey(key);

            case ScreenMode::HighScore:
                return key == -1
                    ? UiEvent::None
                    : UiEvent::ReturnToMenu;
        }

        return UiEvent::None;
    }

    // Ve giao dien.
    void render(
        int tick,
        const std::string& statusMessage
    ) {
        compose(tick);
        drawCanvas();
        drawFooter(statusMessage);

        std::cout << std::flush;
    }

private:

    Canvas canvas_{};

    ScreenMode screen_;

    int selectedRetryOption_ = 0;
    // 0 = Yes, 1 = No.

    int lastScore_ = 0;
    int bestScore_ = 0;

    // ========================================================================
    // CLEAR CANVAS
    // ========================================================================

    void clearCanvas() {
        for (auto& row : canvas_) {
            for (auto& cell : row) {
                cell = Cell{
                    " ",
                    "#ffffff",
                    COLOR_NAVY
                };
            }
        }
    }

    // ========================================================================
    // DRAW TEXT
    // ========================================================================

    void put(
        int row,
        int column,
        const std::string& value,
        const std::string& foreground,
        const std::string& background
    ) {
        const auto characters = splitUtf8(value);

        for (std::size_t i = 0; i < characters.size(); ++i) {
            const int targetColumn =
                column + static_cast<int>(i);

            if (row >= 0 &&
                row < BOARD_ROWS &&
                targetColumn >= 0 &&
                targetColumn < SCREEN_COLS - 2) {

                canvas_[row + 1][targetColumn + 1] =
                    Cell{
                        characters[i],
                        foreground,
                        background
                    };
            }
        }
    }

    // ========================================================================
    // FILL RECTANGLE
    // ========================================================================

    void fill(
        int firstRow,
        int lastRow,
        int firstColumn,
        int lastColumn,
        const std::string& background
    ) {
        for (int row = firstRow; row <= lastRow; ++row) {
            if (row < 0 || row >= BOARD_ROWS) {
                continue;
            }

            const int width =
                std::max(0, lastColumn - firstColumn + 1);

            put(
                row,
                firstColumn,
                std::string(
                    static_cast<std::size_t>(width),
                    ' '
                ),
                "#ffffff",
                background
            );
        }
    }

    // ========================================================================
    // CENTER TEXT
    // ========================================================================

    void putCentered(
        int row,
        const std::string& value,
        const std::string& foreground,
        const std::string& background = COLOR_NAVY
    ) {
        const int column =
            (SCREEN_COLS - textLength(value)) / 2;

        put(
            row,
            column,
            value,
            foreground,
            background
        );
    }

    // ========================================================================
    // FRAME AND BACKGROUND
    // ========================================================================

    void drawFrameAndBackground() {

        // Original dotted background.
        for (int row = 0; row < BOARD_ROWS; ++row) {
            for (int column = 0; column < BOARD_COLS; ++column) {

                put(
                    row,
                    column * 2,
                    ".",
                    COLOR_DOT,
                    COLOR_BOARD
                );

                put(
                    row,
                    column * 2 + 1,
                    " ",
                    COLOR_DOT,
                    COLOR_BOARD
                );
            }
        }

        // Outer horizontal frame.
        for (int column = 1;
             column < SCREEN_COLS - 1;
             ++column) {

            canvas_[0][column] =
                Cell{"─", COLOR_FRAME, COLOR_NAVY};

            canvas_[SCREEN_ROWS - 1][column] =
                Cell{"─", COLOR_FRAME, COLOR_NAVY};
        }

        // Outer vertical frame.
        for (int row = 1;
             row < SCREEN_ROWS - 1;
             ++row) {

            canvas_[row][0] =
                Cell{"│", COLOR_FRAME, COLOR_NAVY};

            canvas_[row][SCREEN_COLS - 1] =
                Cell{"│", COLOR_FRAME, COLOR_NAVY};
        }

        // Four corners.
        canvas_[0][0] =
            Cell{"╭", COLOR_FRAME, COLOR_NAVY};

        canvas_[0][SCREEN_COLS - 1] =
            Cell{"╮", COLOR_FRAME, COLOR_NAVY};

        canvas_[SCREEN_ROWS - 1][0] =
            Cell{"╰", COLOR_FRAME, COLOR_NAVY};

        canvas_[SCREEN_ROWS - 1][SCREEN_COLS - 1] =
            Cell{"╯", COLOR_FRAME, COLOR_NAVY};
    }

    // ========================================================================
    // GAME OVER SCREEN
    // GIU NGUYEN BO CUC VA MAU SAC TU GAMEOVER.CPP BAN DA GUI.
    // ========================================================================

    void drawGameOver(int tick) {

        fill(
            5,
            16,
            10,
            37,
            COLOR_NAVY
        );

        // Top and bottom borders.
        put(
            5,
            10,
            "╭" + repeatCharacter("─", 26) + "╮",
            COLOR_BOX_BORDER,
            COLOR_NAVY
        );

        put(
            16,
            10,
            "╰" + repeatCharacter("─", 26) + "╯",
            COLOR_BOX_BORDER,
            COLOR_NAVY
        );

        // Side borders.
        for (int row = 6; row <= 15; ++row) {
            put(
                row,
                10,
                "│",
                COLOR_BOX_BORDER,
                COLOR_NAVY
            );

            put(
                row,
                37,
                "│",
                COLOR_BOX_BORDER,
                COLOR_NAVY
            );
        }

        // Blinking GAME OVER title.
        const bool blink = ((tick / 4) % 2) != 0;

        putCentered(
            7,
            "G A M E   O V E R",
            blink ? COLOR_RED : "#e02020"
        );

        // Last score.
        putCentered(
            9,
            "Previous score: " + std::to_string(lastScore_),
            COLOR_YELLOW
        );

        // High score / new best.
        if (lastScore_ > bestScore_) {

            if (blink) {
                putCentered(
                    10,
                    "* NEW BEST! *",
                    COLOR_GREEN
                );
            }

        } else {

            putCentered(
                10,
                "Best score: " + std::to_string(bestScore_),
                COLOR_TEXT_DIM
            );
        }

        putCentered(
            12,
            "Retry?",
            "#ffffff"
        );

        // Yes / No labels.
        const std::array<std::string, 2> selectedLabels = {
            "▶ Yes ◀",
            "▶ No ◀"
        };

        const std::array<std::string, 2> idleLabels = {
            "  Yes  ",
            "  No  "
        };

        const std::array<int, 2> columns = {
            14,
            26
        };

        const std::array<int, 2> widths = {
            9,
            8
        };

        const std::array<std::string, 2> activeColors = {
            COLOR_GREEN,
            COLOR_RED
        };

        for (int i = 0; i < 2; ++i) {

            if (i == selectedRetryOption_) {

                fill(
                    14,
                    14,
                    columns[i],
                    columns[i] + widths[i] - 1,
                    COLOR_BOX_ACTIVE
                );

                put(
                    14,
                    columns[i] + 1,
                    selectedLabels[i],
                    activeColors[i],
                    COLOR_BOX_ACTIVE
                );

            } else {

                put(
                    14,
                    columns[i] + 1,
                    idleLabels[i],
                    COLOR_TEXT_DIM,
                    COLOR_NAVY
                );
            }
        }

        // Help text.
        putCentered(
            20,
            " A/D: Move   Enter: Select ",
            COLOR_TEXT_HELP,
            COLOR_BOARD
        );

        putCentered(
            21,
            " Y = Yes   N = No ",
            COLOR_TEXT_HELP,
            COLOR_BOARD
        );
    }

    // ========================================================================
    // PAUSE SCREEN
    // ========================================================================

    void drawPaused() {

        fill(
            5,
            16,
            10,
            37,
            COLOR_NAVY
        );

        put(
            5,
            10,
            "╭" + repeatCharacter("─", 26) + "╮",
            COLOR_BOX_BORDER,
            COLOR_NAVY
        );

        put(
            16,
            10,
            "╰" + repeatCharacter("─", 26) + "╯",
            COLOR_BOX_BORDER,
            COLOR_NAVY
        );

        for (int row = 6; row <= 15; ++row) {

            put(
                row,
                10,
                "│",
                COLOR_BOX_BORDER,
                COLOR_NAVY
            );

            put(
                row,
                37,
                "│",
                COLOR_BOX_BORDER,
                COLOR_NAVY
            );
        }

        putCentered(
            7,
            "P A U S E D",
            COLOR_YELLOW
        );

        putCentered(
            9,
            "Current score: " + std::to_string(lastScore_),
            COLOR_TEXT_DIM
        );

        putCentered(
            12,
            "P / Enter: Resume",
            "#ffffff"
        );

        putCentered(
            14,
            "M: Main Menu",
            COLOR_TEXT_HELP
        );
    }

    // ========================================================================
    // HIGH SCORE SCREEN
    // ========================================================================

    void drawHighScore() {

        fill(
            5,
            16,
            10,
            37,
            COLOR_NAVY
        );

        put(
            5,
            10,
            "╭" + repeatCharacter("─", 26) + "╮",
            COLOR_BOX_BORDER,
            COLOR_NAVY
        );

        put(
            16,
            10,
            "╰" + repeatCharacter("─", 26) + "╯",
            COLOR_BOX_BORDER,
            COLOR_NAVY
        );

        for (int row = 6; row <= 15; ++row) {

            put(
                row,
                10,
                "│",
                COLOR_BOX_BORDER,
                COLOR_NAVY
            );

            put(
                row,
                37,
                "│",
                COLOR_BOX_BORDER,
                COLOR_NAVY
            );
        }

        putCentered(
            8,
            "H I G H   S C O R E",
            COLOR_YELLOW
        );

        putCentered(
            11,
            std::to_string(bestScore_),
            COLOR_GREEN
        );

        putCentered(
            14,
            "Press Enter to return",
            COLOR_TEXT_HELP
        );
    }

    // ========================================================================
    // COMPOSE SCREEN
    // ========================================================================

    void compose(int tick) {

        clearCanvas();

        drawFrameAndBackground();

        switch (screen_) {

            case ScreenMode::GameOver:
                drawGameOver(tick);
                break;

            case ScreenMode::Paused:
                drawPaused();
                break;

            case ScreenMode::HighScore:
                drawHighScore();
                break;
        }
    }

    // ========================================================================
    // HEX COLOR TO RGB
    // ========================================================================

    std::string toRgb(const std::string& hexColor) const {

        if (hexColor.size() != 7 ||
            hexColor[0] != '#') {
            return "255;255;255";
        }

        const int red = std::stoi(
            hexColor.substr(1, 2),
            nullptr,
            16
        );

        const int green = std::stoi(
            hexColor.substr(3, 2),
            nullptr,
            16
        );

        const int blue = std::stoi(
            hexColor.substr(5, 2),
            nullptr,
            16
        );

        return std::to_string(red) + ";" +
               std::to_string(green) + ";" +
               std::to_string(blue);
    }

    // ========================================================================
    // DRAW CANVAS
    // ========================================================================

    void drawCanvas() const {

        std::cout << "\x1b[H";

        std::string previousForeground;
        std::string previousBackground;

        bool firstCell = true;

        for (int row = 0; row < SCREEN_ROWS; ++row) {

            for (int column = 0;
                 column < SCREEN_COLS;
                 ++column) {

                const Cell& cell =
                    canvas_[row][column];

                if (firstCell ||
                    cell.foreground != previousForeground ||
                    cell.background != previousBackground) {

                    std::cout
                        << "\x1b[38;2;"
                        << toRgb(cell.foreground)
                        << ";48;2;"
                        << toRgb(cell.background)
                        << 'm';

                    previousForeground = cell.foreground;
                    previousBackground = cell.background;

                    firstCell = false;
                }

                std::cout << cell.character;
            }

            if (row != SCREEN_ROWS - 1) {
                std::cout << "\r\n";
            }
        }

        std::cout << "\x1b[0m\r\n";
    }

    // ========================================================================
    // FOOTER
    // ========================================================================

    void drawFooter(
        const std::string& statusMessage
    ) const {

        std::string message = statusMessage;

        if (message.empty()) {

            switch (screen_) {

                case ScreenMode::GameOver:
                    message =
                        "Game Over: A/D or Left/Right, Enter, "
                        "Y = Yes, N = No, M = Menu.";
                    break;

                case ScreenMode::Paused:
                    message =
                        "Paused: P / Enter = Resume, M = Main Menu.";
                    break;

                case ScreenMode::HighScore:
                    message =
                        "High Score: Enter / any key = return to Menu.";
                    break;
            }
        }

        std::cout
            << "\x1b[" << SCREEN_ROWS + 1 << ";1H"
            << "\x1b[38;2;154;166;224m"
            << message
            << "\x1b[K";
    }

    // ========================================================================
    // GAME OVER INPUT
    // ========================================================================

    UiEvent handleGameOverKey(int key) {

        // A/D or arrow keys: switch between Yes and No.
        if (key == KEY_LEFT ||
            key == KEY_RIGHT ||
            key == 'a' ||
            key == 'A' ||
            key == 'd' ||
            key == 'D') {

            selectedRetryOption_ =
                1 - selectedRetryOption_;

        }

        // Enter: confirm current selection.
        else if (key == 13 ||
                 key == 10 ||
                 key == ' ') {

            if (selectedRetryOption_ == 0) {
                return UiEvent::RetrySelected;
            }

            return UiEvent::ReturnToMenu;
        }

        // Y: retry immediately.
        else if (key == 'y' || key == 'Y') {

            selectedRetryOption_ = 0;

            return UiEvent::RetrySelected;
        }

        // N or M: return to Main Menu.
        else if (key == 'n' ||
                 key == 'N' ||
                 key == 'm' ||
                 key == 'M') {

            selectedRetryOption_ = 1;

            return UiEvent::ReturnToMenu;
        }

        return UiEvent::None;
    }

    // ========================================================================
    // PAUSE INPUT
    // ========================================================================

    UiEvent handlePauseKey(int key) {

        // P / R / Enter / Space: resume.
        if (key == 'p' ||
            key == 'P' ||
            key == 'r' ||
            key == 'R' ||
            key == 13 ||
            key == 10 ||
            key == ' ') {

            return UiEvent::ResumeSelected;
        }

        // M / ESC: return to Main Menu.
        if (key == 'm' ||
            key == 'M' ||
            key == KEY_ESCAPE) {

            return UiEvent::ReturnToMenu;
        }

        return UiEvent::None;
    }
};

// ============================================================================
// GENERIC SCREEN LOOP
// ============================================================================

UiEvent runScreen(
    GameOverUI& ui,
    const std::string& prompt
) {
    GameOverTerminal terminal;

    std::cout << "\x1b[2J\x1b[H" << std::flush;

    int tick = 0;

    while (true) {

        ui.render(
            tick++,
            prompt
        );

        const int key = readUiKey();

        if (key != -1) {

            const UiEvent event =
                ui.handleKey(key);

            if (event != UiEvent::None) {
                return event;
            }
        }

        std::this_thread::sleep_for(
            std::chrono::milliseconds(40)
        );
    }
}

} // namespace

// ============================================================================
// PUBLIC FUNCTIONS
// GAME ENGINE CALLS THESE FUNCTIONS.
// ============================================================================

// Game Over screen.
UiEvent showGameOverScreen(
    int lastScore,
    int highScore
) {
    GameOverUI ui(
        ScreenMode::GameOver,
        lastScore,
        highScore
    );

    return runScreen(
        ui,
        "Game Over: Choose Yes to retry or No to return to Menu."
    );
}

// Pause screen.
UiEvent showPauseScreen(
    int currentScore,
    int highScore
) {
    GameOverUI ui(
        ScreenMode::Paused,
        currentScore,
        highScore
    );

    return runScreen(
        ui,
        "Paused: Press P or Enter to resume; M to return to Menu."
    );
}

// High Score screen.
void showHighScoreScreen(int highScore) {

    GameOverUI ui(
        ScreenMode::HighScore,
        0,
        highScore
    );

    (void)runScreen(
        ui,
        "High Score: Press any key to return to Menu."
    );
}

} // namespace TetrisUI
