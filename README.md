# SS004-Final

## Source architecture

The playable implementation is split into focused modules under `src`:

- `Board`: stores the playfield, collision checks, locking, and line clearing.
- `Piece`: stores tetrominoes and applies clockwise 90-degree rotation.
- `Game`: coordinates movement, hard drop, locking, game-over detection, and
  wall kicks of up to two cells.
- `Input`, `Renderer`, and `Score`: handle keyboard commands, console output,
  and scoring.

Controls: `A`/`D` move left/right, `X` performs a hard drop, `W` rotates, and
`Q` quits. The original sample remains unchanged in `legacy/Tetris.cpp`.