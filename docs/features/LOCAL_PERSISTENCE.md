# Local persistence

`app/main.js` stores the highest score under
`block-bloom-high-score` using browser `localStorage`. It is local-only and no
network request is made. Board state, settings, lifetime statistics, and
replays are not persisted.
