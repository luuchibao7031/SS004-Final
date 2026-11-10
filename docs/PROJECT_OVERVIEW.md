# Project overview

## Goal and scope

Block Bloom is a local-first Tetris learning project. Its primary deliverable
is a dependency-free browser game with a bright responsive interface. The
repository also preserves the original C++ console implementation for
behavior comparison and academic regression testing.

The scope includes board mechanics, seven tetrominoes, movement, rotation,
gravity, locking, line clearing, scoring, level progression, next preview,
ghost projection, pause/restart, local high score, and presentation effects.

## Technology and platform

- Browser: HTML, CSS, and native JavaScript ES modules.
- Local server: Python's standard-library `http.server` or an equivalent
  static server.
- Preserved baseline: C++17 source using the console/Windows `_conio.h` path.
- No package manager, external service, audio library, or bundler.

## Technical decisions

Game rules are isolated in `app/game.js`; `app/main.js` maps state to DOM and
effects; `app/styles.css` owns visual presentation. This keeps animation and
layout from changing collision, scoring, or line-clear behavior.

## Limits and future work

The app has no hold mechanic, audio system, production bundle, automated
browser test suite, or executable packaging. Possible future work includes
hold, configurable themes, optional sound effects, seeded replay tests, and a
browser test runner.
