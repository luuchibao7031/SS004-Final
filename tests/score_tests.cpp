
#include "../src/Score.h"

#include <cassert>
#include <iostream>

int main() {
    // 1. Kiem tra gia tri khoi tao
    Score score;

    assert(score.value() == 0);
    assert(score.lines() == 0);
    assert(score.level() == 1);
    assert(score.combo() == 0);
    assert(score.fallIntervalMs() == 500);

    // 2. Kiem tra tinh diem va combo
    score.addLines(1);

    assert(score.value() == 100);
    assert(score.lines() == 1);
    assert(score.combo() == 1);

    score.addLines(1);

    assert(score.value() == 250);
    assert(score.lines() == 2);
    assert(score.combo() == 2);

    // 3. Kiem tra combo khi xoa nhieu dong
    score.addLines(0);

    assert(score.combo() == 0);
    assert(score.lines() == 2);

    // 4. Kiem tra diem khi xoa 2, 3 va 4 dong
    Score clearTest;

    clearTest.addLines(2);
    assert(clearTest.value() == 300);

    clearTest.addLines(3);
    assert(clearTest.value() == 800);

    clearTest.addLines(4);
    assert(clearTest.value() == 1600);

    // 5. Kiem tra tang Level sau moi 10 dong
    Score levelTest;

    for (int i = 0; i < 10; ++i) {
        levelTest.addLines(1);
    }

    assert(levelTest.lines() == 10);
    assert(levelTest.level() == 2);
    assert(levelTest.fallIntervalMs() == 460);

    // 6. Kiem tra toc do roi tang dan
    for (int i = 0; i < 100; ++i) {
        levelTest.addLines(1);
    }

    assert(levelTest.level() == 11);
    assert(levelTest.fallIntervalMs() == 80);

    // 7. Kiem tra reset
    levelTest.reset();

    assert(levelTest.value() == 0);
    assert(levelTest.lines() == 0);
    assert(levelTest.level() == 1);
    assert(levelTest.combo() == 0);
    assert(levelTest.fallIntervalMs() == 500);

    std::cout << "All Score tests passed!\n";

    return 0;
}
