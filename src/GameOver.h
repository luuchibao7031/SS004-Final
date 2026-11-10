#pragma once

namespace TetrisUI {

enum class UiEvent {
    None,
    RetrySelected,
    ReturnToMenu,
    ResumeSelected
};

UiEvent showGameOverScreen(int lastScore, int highScore);

UiEvent showPauseScreen(
    int currentScore,
    int highScore
);

void showHighScoreScreen(int highScore);

}
