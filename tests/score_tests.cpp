#include "../src/Score.h"

#include <cassert>

int main() {
    Score score;

    score.addLines(1);
    assert(score.value() == 100);
    assert(score.lines() == 1);
    assert(score.level() == 1);

    score.addLines(3);
    assert(score.value() == 600);
    assert(score.lines() == 4);

    score.addLines(6);
    assert(score.value() == 600);
    assert(score.lines() == 4);

    score.addLines(6);
    score.addLines(6);
    score.addLines(6);
    score.addLines(6);
    score.addLines(6);
    score.addLines(6);
    assert(score.level() == 1);

    score.addLines(4);
    score.addLines(4);
    assert(score.lines() == 12);
    assert(score.level() == 2);
    return 0;
}
