# Movement and rotation

## Scope

This branch implements the movement and rotation assignment without changing
[`legacy/Tetris.cpp`](../legacy/Tetris.cpp). The playable code is split into
small modules under [`src/`](../src/):

- [`Board`](../src/Board.h): validates positions, locks pieces, and clears
  completed lines.
- [`Piece`](../src/Piece.h): stores tetromino layouts and rotates them
  clockwise.
- [`Game`](../src/Game.h): coordinates movement, falling, hard drop, locking,
  game-over detection, and wall kicks.
- [`Input`](../src/Input.h): maps keyboard keys to game commands.
- [`Renderer`](../src/Renderer.h): renders the board, active piece, and ghost
  landing position.
- [`Score`](../src/Score.h): calculates line-clear points and level.

## Implemented controls

| Key | Action |
| --- | --- |
| `A` | Move left when the destination is valid |
| `D` | Move right when the destination is valid |
| `X` | Hard drop until the piece cannot move down |
| `W` | Rotate clockwise and attempt a wall kick |
| `Q` | Quit |

## Collision and rotation

`Board::canPlace` checks every occupied cell of a piece against the inside of
the board and against already locked cells. `Board::dropDistance` reuses this
collision check to calculate the lowest valid row. `Game::canMove` reuses the
same rules for horizontal and vertical movement, so movement, hard drop, and
rotation remain consistent.

Clockwise rotation uses the required matrix transformation:

```text
rotated[row][column] = current[3 - column][row]
```

After creating a rotated piece, `Game::tryRotate` tests horizontal and
vertical offsets around the current position. The rotation is committed only
when one candidate position is valid; otherwise the original orientation and
position remain unchanged.

The renderer displays a ghost piece at the lowest valid landing position.
Every ten cleared lines increase the level and reduce the automatic fall
interval, with an 80 ms lower limit.

## Development and verification process

1. The original sample in `legacy/` was kept read-only for comparison.
2. The single-file implementation was replaced by the modular source layout
   required by the project.
3. Movement and rotation logic were placed in `Game`, `Board`, `Piece`, and
   `Input` instead of duplicating board checks.
4. The implementation was built with MSYS2 UCRT64:

   ```bash
   cd /c/Users/MINH/Documents/SS004/SS004-Final
   g++ -std=c++17 -Wall -Wextra src/*.cpp -o build/tetris.exe
   ```

5. The generated executable is a local build artifact and is excluded by
   [`build/*.exe`](../.gitignore).

The loop is intentionally separated into a responsive frame interval of
approximately 16 ms and an independent 500 ms falling interval. The renderer
also redraws only after a state change and moves the cursor to the top of the
existing console instead of launching `system("cls")` for every frame.

## Manual test checklist

Run `./build/tetris.exe` from MSYS2 UCRT64 and verify:

- `A` and `D` stop at the board walls and occupied cells.
- `X` moves the active piece to its lowest valid row.
- `W` rotates a piece without overwriting the wall or locked cells.
- Rotating near a wall tries the small horizontal offsets; if all fail, the
  piece remains unchanged.
- A piece locks when it can no longer move down.
- A completed line is removed and the score increases.
- The ghost piece marks the landing row and the level increases after ten
  cleared lines, making automatic falling faster.
- `Q` exits without modifying the legacy source.

## Automated checks

The focused movement and rotation checks can be built and run separately from
the game executable:

```bash
g++ -std=c++17 -Wall -Wextra \
  tests/movement_rotation_tests.cpp src/Board.cpp src/Piece.cpp \
  -o build/movement_rotation_tests.exe
./build/movement_rotation_tests.exe
```

The checks cover four clockwise rotations returning a piece to its original
orientation, board-boundary rejection, collision rejection after locking a
piece, and hard-drop distance at the bottom boundary.

Score and level progression are checked separately:

```bash
g++ -std=c++17 -Wall -Wextra \
  tests/score_tests.cpp src/Score.cpp -o build/score_tests.exe
./build/score_tests.exe
```
