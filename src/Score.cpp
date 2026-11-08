#include "Score.h"

void Score::addLines(int lines) {
    static constexpr int linePoints[] = {0, 100, 300, 500, 800};
    if (lines >= 1 && lines <= 4) {
        value_ += linePoints[lines];
    }
}

int Score::value() const {
    return value_;
}
