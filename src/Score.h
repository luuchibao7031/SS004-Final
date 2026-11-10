#pragma once

class Score {
public:
    void addLines(int lines);
    int value() const;
    int lines() const;
    int level() const;

private:
    int value_ = 0;
    int lines_ = 0;
};