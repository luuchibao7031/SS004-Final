# Invisible mode

Invisible mode is selected from the `☷ Mode` menu. The active piece remains
visible, while locked cells and the ghost projection are hidden after they
settle. Collision detection, locking, scoring, and line clearing continue to
use the real board.

When a line is cleared, the core keeps a pre-clear snapshot. The presentation
layer alternates that snapshot on and off for a short flash, then hides locked
cells again. The snapshot is never written back to the game board.
