# Feature audit

| Feature | Status | Evidence |
| --- | --- | --- |
| Board, pieces, movement, rotation, gravity, locking | Completed | `app/game.js` |
| Scoring and level progression | Completed | `Score` in `app/game.js` |
| Next preview | Completed | `Game.next`, `main.js`, `index.html` |
| Ghost piece | Completed | `main.js` projection |
| Pause/resume and restart | Completed | `main.js` and overlay |
| Local high score | Completed | `localStorage` in `main.js` |
| Sound effects and mute control | Completed | `app/sound.js`, local OGG assets |
| Invisible mode | Completed | `main.js` presentation toggle and clear flash |
| Line-clear popup and particles | Completed | `celebrate` in `main.js` |
| Light responsive theme | Completed | `styles.css` |
| Hold piece | Not Implemented | UI intentionally displays classic-rules placeholder |
| Lifetime statistics/settings persistence | Not Implemented | only high score is stored |
| Production bundling/executable packaging | Not Implemented | static files are served directly |
