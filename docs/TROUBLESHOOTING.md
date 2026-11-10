# Troubleshooting

## Blank page or module error

Serve `app/` over HTTP rather than opening `index.html` directly:

```text
python -m http.server 8000 --directory app
```

Check that `app/index.html`, `app/main.js`, `app/game.js`, and
`app/styles.css` are in the same directory.

## Controls do not respond

Click the page, use the documented A/D/W/S/X keys or arrow keys, and check
the browser console for module-loading errors. Pause intentionally blocks game
commands.

## High score does not persist

The browser must allow localStorage for the origin. Private browsing,
storage quotas, or restrictive browser policies can disable persistence; the
game remains playable with a temporary high score.

## C++ build fails

Use an MSYS2 UCRT64 terminal with a C++17-capable `g++`. The browser app does
not require the C++ toolchain. Build outputs are generated under `build/` and
are ignored by Git.

## Visual or animation issues

Use a modern browser and inspect the console. Users with reduced-motion
enabled receive simplified animations by design. No external assets or audio
files need to be loaded.
