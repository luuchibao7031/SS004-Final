#include "Game.h"

#include "Input.h"
#include "Renderer.h"

#include <chrono>
#include <algorithm>
#include <iostream>
#include <thread>

Game::Game()
    : activePiece_(Piece::random()), nextPiece_(Piece::random()),
      pieceX_(5), pieceY_(1) {}

bool Game::canMove(int deltaX, int deltaY) const {
    return board_.canPlace(activePiece_, pieceX_ + deltaX, pieceY_ + deltaY);
}

bool Game::tryRotate() {
    const Piece rotated = activePiece_.rotatedClockwise();
    struct Kick {
        int x;
        int y;
    };
    constexpr Kick wallKickOffsets[] = {
        {0, 0}, {-1, 0}, {1, 0}, {-2, 0}, {2, 0},
        {0, -1}, {0, 1}, {-1, -1}, {1, -1}};
    for (const Kick& kick : wallKickOffsets) {
        if (board_.canPlace(rotated, pieceX_ + kick.x, pieceY_ + kick.y)) {
            activePiece_ = rotated;
            pieceX_ += kick.x;
            pieceY_ += kick.y;
            return true;
        }
    }
    return false;
}

void Game::spawnPiece() {
    // TV1: khi khoi hien tai ha xuong, khoi tiep theo tro thanh khoi hien tai
    activePiece_ = nextPiece_;
    nextPiece_   = Piece::random();
    pieceX_ = 5;
    pieceY_ = 1;
}

void Game::lockActivePiece() {
    board_.lock(activePiece_, pieceX_, pieceY_);
    score_.addLines(board_.clearCompletedLines());
    spawnPiece();
}

bool Game::handleCommand(bool& stateChanged) {
    switch (Input::readCommand()) {
    case Command::MoveLeft:
        if (canMove(-1, 0)) {
            --pieceX_;
            stateChanged = true;
        }
        break;
    case Command::MoveRight:
        if (canMove(1, 0)) {
            ++pieceX_;
            stateChanged = true;
        }
        break;
    case Command::HardDrop: {
        const int dropDistance =
            board_.dropDistance(activePiece_, pieceX_, pieceY_);
        pieceY_ += dropDistance;
        if (dropDistance > 0) {
            stateChanged = true;
        }
        break;
    }
    case Command::Rotate:
        stateChanged = tryRotate() || stateChanged;
        break;
    case Command::Quit:
        return false;
    case Command::None:
        break;
    }
    return true;
}

void Game::run() {
    using Clock = std::chrono::steady_clock;
    constexpr auto FrameInterval = std::chrono::milliseconds(16);

    auto lastFall = Clock::now();
    bool stateChanged = true;

    while (true) {
        if (!handleCommand(stateChanged)) {
            return;
        }

        const auto now = Clock::now();
        const auto fallInterval = std::chrono::milliseconds(
            std::max(80, 500 - (score_.level() - 1) * 40));
        if (now - lastFall >= fallInterval) {
            if (canMove(0, 1)) {
                ++pieceY_;
                stateChanged = true;
            } else {
                lockActivePiece();
                stateChanged = true;
                if (!board_.canPlace(activePiece_, pieceX_, pieceY_)) {
                    Renderer::draw(board_, activePiece_, pieceX_, pieceY_,
                                   score_.value(), score_.level(), nextPiece_);
                    std::cout << "Game over!\n";
                    return;
                }
            }
            lastFall = now;
        }

        if (stateChanged) {
            Renderer::draw(board_, activePiece_, pieceX_, pieceY_,
                           score_.value(), score_.level(), nextPiece_);
            stateChanged = false;
        }
        std::this_thread::sleep_for(FrameInterval);
    }
}
