#pragma once

class Score {
public:
    void addLines(int lines);
    int value() const;

private:
    int value_ = 0;
};
