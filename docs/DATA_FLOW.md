# Data flow

1. A keyboard event reaches `app/main.js`.
2. The input mapping calls a `Game` command (`move`, `rotate`, `hardDrop`) or
   toggles pause/restart.
3. `Game` asks `Board.canPlace` before movement or rotation.
4. Gravity is evaluated from `requestAnimationFrame` timestamps; a blocked
   fall calls `Game.lock`.
5. `Board.lock` mutates locked cells, then `clearCompletedLines` returns row
   indexes.
6. `Score.addLines` updates points, line count, and derived level.
7. `main.js` renders the board and ghost projection and draws the next piece.
8. A non-empty clear result triggers popup/particle effects and nothing else.
9. High score is the only persisted value, stored under
   `block-bloom-high-score` in browser localStorage.
10. Input and line-clear/game-over events call `app/sound.js`, which plays
    local OGG files when sound is enabled. The mute preference is stored under
    `block-bloom-sound-enabled`.
11. Invisible mode masks locked board cells during rendering. On a clear, the
    pre-clear snapshot is shown on a short alternating flash before masking
    resumes; the core board remains authoritative throughout.

The core never owns DOM, storage, animation, or audio. There is no lifetime
statistics store, settings store, or server boundary.
