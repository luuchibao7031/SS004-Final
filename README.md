# SS004-Final

## Source architecture

The playable implementation is split into focused modules under `src`:

- `Board`: stores the playfield, collision checks, locking, and line clearing.
- `Piece`: stores tetrominoes and applies clockwise 90-degree rotation.
- `Game`: coordinates movement, hard drop, locking, game-over detection, and
  horizontal/vertical wall kicks.
- `Input`, `Renderer`, and `Score`: handle keyboard commands, console output,
  scoring, level progression, and the ghost landing position.

Controls: `A`/`D` move left/right, `X` performs a hard drop, `W` rotates, and
`Q` quits. The original sample remains unchanged in `legacy/Tetris.cpp`.

## Build and run

Install the MSYS2 UCRT64 GCC toolchain, open the **MSYS2 UCRT64** terminal,
then run:

```bash
cd /c/Users/MINH/Documents/SS004/SS004-Final
g++ -std=c++17 -Wall -Wextra src/*.cpp -o build/tetris.exe
./build/tetris.exe
```

On macOS, build and run from the project directory with:

```bash
clang++ -std=c++17 -Wall -Wextra \
  -isystem "$(xcrun --show-sdk-path)/usr/include/c++/v1" \
  src/*.cpp -o build/tetris
./build/tetris
```

The implementation details, development process, and manual test checklist
are documented in [`docs/movement-rotation.md`](docs/movement-rotation.md).

The game loop polls input approximately every 16 ms while automatic falling
uses an independent 500 ms timer. Rendering is performed only after a state
change and updates the existing console screen instead of invoking
`system("cls")`, reducing flicker and input delay.

The renderer shows a `.` ghost piece at the lowest valid landing position.
Every 10 cleared lines increases the level and accelerates automatic falling,
down to a safe minimum interval.