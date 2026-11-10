#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#include "Game.h"

#include "Input.h"
#include "Renderer.h"

#include <algorithm>
#include <chrono>
#include <iostream>

#ifdef _WIN32
#include <windows.h>   // Sleep()
#else
#include <thread>
#endif

Game::Game()
    : activePiece_(Piece::random()), nextPiece_(Piece::random()),
      pieceX_((Board::Width - Piece::Size) / 2), pieceY_(1) {}

bool Game::canMove(int deltaX, int deltaY) const {
    return board_.canPlace(activePiece_, pieceX_ + deltaX, pieceY_ + deltaY);
}

bool Game::tryRotate() {
    const Piece rotated = activePiece_.rotatedClockwise();

    // Wall kick don gian: thu cac vi tri gan nhat neu xoay tai cho bi chan.
    struct Kick {
        int x;
        int y;
    };
    constexpr Kick offsets[] = {
        {0, 0}, {-1, 0}, {1, 0}, {-2, 0}, {2, 0},
        {0, -1}, {0, 1}, {-1, -1}, {1, -1}
    };

    for (const Kick& offset : offsets) {
        const int newX = pieceX_ + offset.x;
        const int newY = pieceY_ + offset.y;

        if (board_.canPlace(rotated, newX, newY)) {
            activePiece_ = rotated;
            pieceX_ = newX;
            pieceY_ = newY;
            return true;
        }
    }
    return false;
}

void Game::spawnPiece() {
    activePiece_ = nextPiece_;
    nextPiece_   = Piece::random();
    pieceX_ = (Board::Width - Piece::Size) / 2;
    pieceY_ = 1;
}

void Game::lockActivePiece() {
    board_.lock(activePiece_, pieceX_, pieceY_);

    // TV4: Board tra ve so dong da xoa de Score cap nhat diem/level/combo.
    const int clearedLines = board_.clearCompletedLines();
    score_.addLines(clearedLines);

    spawnPiece();
}

bool Game::isGameOver() const {
    return !board_.canPlace(activePiece_, pieceX_, pieceY_);
}

void Game::render() const {
    Renderer::draw(board_, activePiece_, pieceX_, pieceY_,
                   score_.value(), score_.level(), nextPiece_,
                   score_.combo(), score_.lastClearLines());
}

bool Game::handleCommand(bool& stateChanged, bool& pieceLocked) {
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
        const int distance =
            board_.dropDistance(activePiece_, pieceX_, pieceY_);
        pieceY_ += distance;

        // Hard Drop: khoa khoi ngay tai vi tri roi thap nhat.
        lockActivePiece();
        pieceLocked = true;
        stateChanged = true;
        break;
    }

    case Command::Rotate:
        if (tryRotate()) {
            stateChanged = true;
        }
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

    auto lastFall = Clock::now();
    bool stateChanged = true;

    // Ve khung cuoi roi bao het game.
    auto showGameOver = [this]() {
        render();
        std::cout << "\nGame over!\n";
    };

    while (true) {
        bool pieceLocked = false;

        if (!handleCommand(stateChanged, pieceLocked)) {
            return;
        }

        // Bat dau lai dong ho roi sau khi Hard Drop tao khoi moi.
        if (pieceLocked) {
            lastFall = Clock::now();
        }

        // Kiem tra ngay sau khi sinh khoi moi, ke ca sau Hard Drop.
        if (isGameOver()) {
            showGameOver();
            return;
        }

        // TV4: toc do roi tang dan theo level (500 ms -> toi thieu 80 ms).
        const auto now = Clock::now();
        const auto fallInterval =
            std::chrono::milliseconds(score_.fallIntervalMs());

        if (now - lastFall >= fallInterval) {
            if (canMove(0, 1)) {
                ++pieceY_;
                stateChanged = true;
            } else {
                lockActivePiece();
                stateChanged = true;

                if (isGameOver()) {
                    showGameOver();
                    return;
                }
            }
            lastFall = now;
        }

        if (stateChanged) {
            render();
            stateChanged = false;
        }

        // TV4: Sleep() dieu phoi nhip vong lap, thoi gian nghi giam theo level
        // (16 ms o level 1 -> 2 ms tu level 12). Thoi diem khoi roi van do
        // steady_clock + fallIntervalMs() quyet dinh, nen do phan giai cua
        // Sleep() tren Windows khong lam sai toc do roi.
        const int pollMs = score_.pollIntervalMs();
#ifdef _WIN32
        Sleep(static_cast<DWORD>(pollMs));
#else
        std::this_thread::sleep_for(std::chrono::milliseconds(pollMs));
#endif
    }
}
