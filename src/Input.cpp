#include "Input.h"

#include <stdexcept>

#ifdef _WIN32
#include <conio.h>
#else
#include <cerrno>
#include <cstdio>
#include <termios.h>
#include <unistd.h>

namespace {

class TerminalMode {
public:
    TerminalMode() {
        if (!::isatty(STDIN_FILENO)) {
            return;
        }
        if (::tcgetattr(STDIN_FILENO, &original_) == -1) {
            throw std::runtime_error("Unable to read terminal settings");
        }

        termios inputMode = original_;
        inputMode.c_lflag &= static_cast<tcflag_t>(~(ICANON | ECHO));
        inputMode.c_cc[VMIN] = 0;
        inputMode.c_cc[VTIME] = 0;
        if (::tcsetattr(STDIN_FILENO, TCSANOW, &inputMode) == -1) {
            throw std::runtime_error("Unable to enable interactive terminal input");
        }
        active_ = true;
    }

    ~TerminalMode() {
        if (active_ && ::tcsetattr(STDIN_FILENO, TCSANOW, &original_) == -1) {
            std::perror("Unable to restore terminal settings");
        }
    }

    bool active() const {
        return active_;
    }

private:
    termios original_{};
    bool active_ = false;
};

}  // namespace
#endif

Command Input::readCommand() {
    char key;
#ifdef _WIN32
    if (!_kbhit()) {
        return Command::None;
    }
    key = static_cast<char>(_getch());
#else
    static TerminalMode terminalMode;
    if (!terminalMode.active()) {
        return Command::None;
    }

    const ssize_t bytesRead = ::read(STDIN_FILENO, &key, 1);
    if (bytesRead == 0 || (bytesRead == -1 && errno == EINTR)) {
        return Command::None;
    }
    if (bytesRead == -1) {
        throw std::runtime_error("Unable to read input from terminal");
    }
#endif

    switch (key) {
    case 'a':
    case 'A':
        return Command::MoveLeft;
    case 'd':
    case 'D':
        return Command::MoveRight;
    case 'x':
    case 'X':
        return Command::HardDrop;
    case 'w':
    case 'W':
        return Command::Rotate;
    case 'q':
    case 'Q':
        return Command::Quit;
    default:
        return Command::None;
    }
}
