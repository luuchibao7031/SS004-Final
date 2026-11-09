#include "Input.h"

#include <conio.h>

Command Input::readCommand() {
    if (!_kbhit()) {
        return Command::None;
    }

    const char key = static_cast<char>(_getch());
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
