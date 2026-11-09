# Game design

## Board and pieces

The board is 20 rows by 15 columns. The outer row/columns are sentinel walls;
the playable area is 13 by 18. Each tetromino is a 4x4 grid using the
seven `I/O/T/S/Z/J/L` templates.

## Rules

- Spawn starts near column 5 and row 1.
- Left/right movement succeeds only when every occupied cell is valid.
- Soft drop attempts one row.
- Hard drop uses `Board.dropDistance` and locks immediately.
- Rotation is clockwise using the 4x4 matrix transform and the existing nine
  wall-kick candidates.
- A blocked downward move locks the piece, clears complete rows, and spawns
  the next piece.
- Game over occurs when the spawned piece cannot be placed.

## Scoring and levels

Line clears award 100, 300, 500, or 800 points for one through four rows.
Level is `floor(lines / 10) + 1`; gravity starts at 500 ms and decreases by
40 ms per level to a minimum of 80 ms.

## Presentation features

The ghost piece is a projection only. Popup text and particles occur after
the core reports cleared rows. They do not affect scoring or board state.

There is a next preview. Hold, combo scoring, audio, and hard-drop animation
are not implemented; the current UI labels the hold area as classic rules
instead of claiming a hold mechanic.
