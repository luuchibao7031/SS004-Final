#include "Game.h"

#include "Input.h"
#include "Renderer.h"

#include <chrono>
#include <iostream>
#include <thread>

Game::Game()
    : activePiece_(Piece::random()), pieceX_(5), pieceY_(1) {}

bool Game::canMove(int deltaX, int deltaY) const {
    return board_.canPlace(activePiece_, pieceX_ + deltaX, pieceY_ + deltaY);
}

bool Game::tryRotate() {
    const Piece rotated = activePiece_.rotatedClockwise();
    constexpr int wallKickOffsets[] = {0, -1, 1, -2, 2};
    for (const int offset : wallKickOffsets) {
        if (board_.canPlace(rotated, pieceX_ + offset, pieceY_)) {
            activePiece_ = rotated;
            pieceX_ += offset;
            return true;
        }
    }
    return false;
}

void Game::spawnPiece() {
    activePiece_ = Piece::random();
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
    case Command::HardDrop:
        while (canMove(0, 1)) {
            ++pieceY_;
            stateChanged = true;
        }
        break;
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
    constexpr auto FallInterval = std::chrono::milliseconds(500);
    constexpr auto FrameInterval = std::chrono::milliseconds(16);

    auto lastFall = Clock::now();
    bool stateChanged = true;

    while (true) {
        if (!handleCommand(stateChanged)) {
            return;
        }

        const auto now = Clock::now();
        if (now - lastFall >= FallInterval) {
            if (canMove(0, 1)) {
                ++pieceY_;
                stateChanged = true;
            } else {
                lockActivePiece();
                stateChanged = true;
                if (!board_.canPlace(activePiece_, pieceX_, pieceY_)) {
                    Renderer::draw(board_, activePiece_, pieceX_, pieceY_,
                                   score_.value());
                    std::cout << "Game over!\n";
                    return;
                }
            }
            lastFall = now;
        }

        if (stateChanged) {
            Renderer::draw(board_, activePiece_, pieceX_, pieceY_,
                           score_.value());
            stateChanged = false;
        }
        std::this_thread::sleep_for(FrameInterval);
    }
}
