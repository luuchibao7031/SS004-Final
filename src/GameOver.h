
#pragma once

namespace TetrisUI {

enum class UiEvent {
    None,
    RetrySelected,
    ReturnToMenu,
    ResumeSelected
};

// Game Over: returns RetrySelected or ReturnToMenu.
UiEvent showGameOverScreen(
    int lastScore,
    int highScore
);

// Pause: returns ResumeSelected or ReturnToMenu.
UiEvent showPauseScreen(
    int currentScore,
    int highScore
);

// High Score: returns to Menu when a key is pressed.
void showHighScoreScreen(int highScore);

} // namespace TetrisUI
