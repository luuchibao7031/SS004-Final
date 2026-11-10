#include "Score.h"

#include <algorithm>

namespace {
constexpr int LinePoints[5] = {0, 100, 300, 500, 800};
constexpr int ComboBonusPerStep = 50;
constexpr int LinesPerLevel = 10;
constexpr int InitialFallIntervalMs = 500;
constexpr int FallIntervalDecreaseMs = 40;
constexpr int MinimumFallIntervalMs = 80;
constexpr int PollDivisor = 30;   // Sleep() ~ chu ky roi / 30
}

void Score::addLines(int clearedLines) {
    // Du lieu ngoai pham vi khong duoc phep lam thay doi trang thai diem.
    if (clearedLines < 0 || clearedLines > 4) {
        return;
    }

    lastClearLines_ = clearedLines;

    // Khong xoa dong thi combo bi ngat.
    if (clearedLines == 0) {
        combo_ = 0;
        return;
    }

    // Dung level truoc khi cap nhat tong so dong cua luot nay.
    const int currentLevel = level();
    const int baseScore = LinePoints[clearedLines] * currentLevel;

    // Luot xoa lien tiep dau tien khong co thuong.
    // Luot thu 2 +50, thu 3 +100, thu 4 +150, ... (nhan level).
    const int comboBonus =
        ComboBonusPerStep * combo_ * currentLevel;

    value_ += baseScore + comboBonus;
    lines_ += clearedLines;
    ++combo_;
}

int Score::value() const {
    return value_;
}

int Score::lines() const {
    return lines_;
}

int Score::level() const {
    return lines_ / LinesPerLevel + 1;
}

int Score::combo() const {
    return combo_;
}

int Score::lastClearLines() const {
    return lastClearLines_;
}

int Score::fallIntervalMs() const {
    return std::max(
        MinimumFallIntervalMs,
        InitialFallIntervalMs - (level() - 1) * FallIntervalDecreaseMs);
}

int Score::pollIntervalMs() const {
    return std::max(1, fallIntervalMs() / PollDivisor);
}

void Score::reset() {
    value_ = 0;
    lines_ = 0;
    combo_ = 0;
    lastClearLines_ = 0;
}
