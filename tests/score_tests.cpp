
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
    assert(score.lastClearLines() == 0);
    assert(score.fallIntervalMs() == 500);
    assert(score.pollIntervalMs() == 16);

    // 2. Kiem tra tinh diem va combo
    score.addLines(1);

    assert(score.value() == 100);
    assert(score.lines() == 1);
    assert(score.combo() == 1);
    assert(score.lastClearLines() == 1);

    score.addLines(1);

    assert(score.value() == 250);
    assert(score.lines() == 2);
    assert(score.combo() == 2);
    assert(score.lastClearLines() == 1);

    // 3. Kiem tra combo khi khong xoa dong
    score.addLines(0);

    assert(score.value() == 250);
    assert(score.lines() == 2);
    assert(score.combo() == 0);
    assert(score.lastClearLines() == 0);

    // 4. Kiem tra diem khi xoa 2, 3 va 4 dong
    Score clearTest;

    clearTest.addLines(2);
    assert(clearTest.value() == 300);
    assert(clearTest.combo() == 1);

    clearTest.addLines(3);
    assert(clearTest.value() == 850);
    assert(clearTest.combo() == 2);

    clearTest.addLines(4);
    assert(clearTest.value() == 1750);
    assert(clearTest.combo() == 3);

    // 5. Kiem tra du lieu dau vao khong hop le
    const int oldScore = clearTest.value();
    const int oldLines = clearTest.lines();
    const int oldCombo = clearTest.combo();
    const int oldLastClear = clearTest.lastClearLines();

    clearTest.addLines(-1);
    clearTest.addLines(5);

    assert(clearTest.value() == oldScore);
    assert(clearTest.lines() == oldLines);
    assert(clearTest.combo() == oldCombo);
    assert(clearTest.lastClearLines() == oldLastClear);

    // 6. Kiem tra tang Level sau moi 10 dong
    Score levelTest;

    for (int i = 0; i < 10; ++i) {
        levelTest.addLines(1);
    }

    assert(levelTest.lines() == 10);
    assert(levelTest.level() == 2);
    assert(levelTest.fallIntervalMs() == 460);

    // 7. Kiem tra toc do roi dat muc toi thieu
    for (int i = 0; i < 100; ++i) {
        levelTest.addLines(1);
    }

    assert(levelTest.lines() == 110);
    assert(levelTest.level() == 12);
    assert(levelTest.fallIntervalMs() == 80);
    assert(levelTest.pollIntervalMs() == 2);

    // 8. Kiem tra reset
    levelTest.reset();

    assert(levelTest.value() == 0);
    assert(levelTest.lines() == 0);
    assert(levelTest.level() == 1);
    assert(levelTest.combo() == 0);
    assert(levelTest.lastClearLines() == 0);
    assert(levelTest.fallIntervalMs() == 500);
    assert(levelTest.pollIntervalMs() == 16);

    std::cout << "All Score tests passed!\n";

    return 0;
}

