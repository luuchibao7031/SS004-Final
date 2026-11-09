#include "highscore.h"
#include <fstream>

int loadHighScore() {
    std::ifstream in("highscore.txt");
    int score = 0;
    if (in >> score) return score;
    return 0;
}

void saveHighScore(int score) {
    if (score <= loadHighScore()) return;
    std::ofstream out("highscore.txt");
    out << score;
}