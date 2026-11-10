# Testing

## Available automated tests

The C++ tests are standalone `assert` programs:

```text
g++ -std=c++17 -Wall -Wextra tests/movement_rotation_tests.cpp src/Board.cpp src/Piece.cpp -o build/movement_rotation_tests.exe
g++ -std=c++17 -Wall -Wextra tests/score_tests.cpp src/Score.cpp -o build/score_tests.exe
g++ -std=c++17 -Wall -Wextra src/Board.cpp src/Piece.cpp src/Renderer.cpp tests/renderer_board_tests.cpp -o build/test_renderer.exe
```

Run each resulting executable from the repository root. They cover rotation
round-trips, placement boundaries, collision, drop distance, score/level,
board reset, and renderer invocation.

## Browser validation

The browser smoke test starts a static server and checks page rendering,
board cell count, next-piece cell count, and keyboard actions. It is not a
committed automated test suite.

## Current results

The browser smoke test passed during implementation. `git diff --check`
passed. The C++ tests and Node syntax check were not run in the current
environment because `g++` and `node` were unavailable. This is an explicit
testing gap, not a pass claim.
