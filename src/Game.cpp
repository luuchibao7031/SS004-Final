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

bool Game::handleCommand() {
    switch (Input::readCommand()) {
    case Command::MoveLeft:
        if (canMove(-1, 0)) {
            --pieceX_;
        }
        break;
    case Command::MoveRight:
        if (canMove(1, 0)) {
            ++pieceX_;
        }
        break;
    case Command::HardDrop:
        while (canMove(0, 1)) {
            ++pieceY_;
        }
        break;
    case Command::Rotate:
        tryRotate();
        break;
    case Command::Quit:
        return false;
    case Command::None:
        break;
    }
    return true;
}

void Game::run() {
    while (true) {
        if (!handleCommand()) {
            return;
        }

        if (canMove(0, 1)) {
            ++pieceY_;
        } else {
            lockActivePiece();
            if (!board_.canPlace(activePiece_, pieceX_, pieceY_)) {
                Renderer::draw(board_, activePiece_, pieceX_, pieceY_,
                               score_.value());
                std::cout << "Game over!\n";
                return;
            }
        }

        Renderer::draw(board_, activePiece_, pieceX_, pieceY_, score_.value());
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}
