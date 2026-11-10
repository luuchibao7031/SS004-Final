# Ghost piece

Before rendering, the UI asks `Board.dropDistance` for the active piece's
lowest valid position. It renders that projection with a dashed ghost style.
The ghost is not written into the board and cannot affect collision or score.
