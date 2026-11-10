#pragma once

enum class Command {
    None,
    MoveLeft,
    MoveRight,
    HardDrop,
    Rotate,
    Quit
};

class Input {
public:
    static Command readCommand();
};