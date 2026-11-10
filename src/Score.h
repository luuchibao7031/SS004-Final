#pragma once

// diem so, cap do, toc do roi, combo.
class Score {
public:
    // Cap nhat diem sau moi lan khoa khoi.
    // clearedLines = so dong vua xoa, bang 0 neu khong xoa dong.
    void addLines(int clearedLines);

    int value() const;
    int lines() const;
    int level() const;
    int combo() const;            // so lan khoa khoi lien tiep co xoa dong
    int lastClearLines() const;   // so dong cua lan khoa khoi gan nhat (0 = khong xoa dong)

    // Chu ky roi tu dong theo level (don vi: milliseconds).
    int fallIntervalMs() const;

    // Thoi gian nghi cua vong lap game, dung cho Sleep() (ms). Giam dan theo level.
    int pollIntervalMs() const;

    void reset();

private:
    int value_ = 0;
    int lines_ = 0;
    int combo_ = 0;
    int lastClearLines_ = 0;
};
