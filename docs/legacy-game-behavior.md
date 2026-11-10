# Legacy game behavior

## Scope and source

The original implementation is preserved in
[`legacy/Tetris.cpp`](../legacy/Tetris.cpp). It is a Windows console game with
a 20-row by 15-column board. Rows `0` and `19`, and columns `0` and `14`, are
sentinel walls containing `#`; the playable area is the remaining 13 by 18
cells.

The current modular implementation in [`src/`](../src/) was compared with the
legacy source before the UI work. The web presentation must use the same
board, piece, collision, locking, clearing, and scoring rules.

## Data structures

- The legacy board is a global `char board[20][15]`.
- A tetromino is one entry in a 16-entry 4x4 table. The first seven entries
  contain the actual random piece set (`I`, `O`, `T`, `S`, `Z`, `J`, `L`);
  repeated entries are historical duplicates that are not selected by
  `rand() % 7`.
- `x`, `y`, and `b` identify the active piece position and template.
- Empty cells contain a space. Locked cells contain their tetromino letter.

The modular code keeps these rules in `Board`, `Piece`, `Game`, and `Score`.
`Piece` uses the seven effective templates and stores a 4x4 grid per piece.

## Algorithms and gameplay rules

### Spawn and movement

The legacy piece starts at `x = 5`, `y = 0`, and a random template is chosen.
`canMove(dx, dy)` checks every occupied cell against the inside of the
sentinel walls and rejects occupied destinations. A/D move horizontally and X
moves one row down. The modular game retains the same collision predicate and
spawn column; its playable spawn row is `1` so the sentinel top row is never
overwritten.

### Gravity and locking

The legacy loop attempts one downward move every 500 ms. When the move is
blocked, the active cells are copied into the board, completed rows are
removed, and a fresh random piece is spawned. The modular game retains the
500 ms starting interval, accelerates it by 40 ms per level, and clamps it at
80 ms. It also exposes an explicit hard drop and a next-piece preview; neither
changes the lock predicate.

### Rotation

The legacy source has no rotation operation. The modular version adds clockwise
90-degree matrix rotation:

```text
rotated[row][column] = current[3 - column][row]
```

Rotation is accepted only when the rotated 4x4 piece can be placed. The
modular wall-kick candidates are presentation-independent movement support:
`(0,0), (-1,0), (1,0), (-2,0), (2,0), (0,-1), (0,1), (-1,-1), (1,-1)`.

### Line clearing

Rows are scanned from bottom to top. A row is complete when every playable
column is non-empty. Complete rows are replaced by the row above, the top
playable row is cleared, and the scan repeats at the same index. The modular
`Board::clearCompletedLines()` preserves this algorithm and returns the number
of cleared rows.

### Scoring and level

The legacy source does not score or level. The existing modular behavior,
which is preserved by the new presentation, awards 100/300/500/800 points for
one/two/three/four lines, counts all cleared lines, and sets
`level = lines / 10 + 1`. Visual combo messages and particles are presentation
effects only; they do not alter these values.

### Next piece, game over, and controls

The legacy source has no next or hold piece and exits only on `Q`. The modular
version keeps the same random seven-piece pool, adds a next-piece preview, and
detects game over when the next piece cannot be placed at the spawn position.
It maps `A/D` to horizontal movement, `X` to hard drop, `W` to clockwise
rotation, and `Q` to quit.

## Preservation and intentional changes

Preserved:

- 20x15 board dimensions and sentinel-wall model.
- Seven-piece shapes and their 4x4 geometry.
- Collision and lock behavior.
- Bottom-up line clearing.
- Initial gravity interval and the modular score/level formulas.

Intentional presentation/application changes:

- A local browser UI replaces the console renderer.
- Pause, restart, hold, ghost-piece, and line-clear particles are UI/game
  application features. Pause and restart do not mutate board rules; hold is
  disabled unless explicitly enabled by the application state.
- The legacy source remains unchanged for regression comparison.

The only gameplay capability added by the modular baseline is rotation (with
wall kicks), hard drop, next preview, scoring, and level progression. These
are documented rather than silently attributed to the legacy sample.
