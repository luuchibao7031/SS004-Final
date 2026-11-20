#include "HighScore.h"
#include <fstream>

namespace {
constexpr const char* HIGH_SCORE_FILE = "highscore.txt";
}

int loadHighScore() {
    std::fstream file(HIGH_SCORE_FILE, std::ios::in);

    int score = 0;

    if (!(file >> score) || score < 0) {
        return 0;
    }

    return score;
}

void saveHighScore(int score) {
    if (score < 0 || score <= loadHighScore()) {
        return;
    }

    std::fstream file(
        HIGH_SCORE_FILE,
        std::ios::out | std::ios::trunc
    );

    if (!file.is_open()) return;

    file << score << '\n';
    file.flush();
}
