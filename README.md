# Block Bloom Tetris

Block Bloom is a local-first Tetris learning project. The playable deliverable
is a dependency-free browser game with a bright responsive interface. The
original C++ console implementation remains in the repository as a legacy
reference and regression baseline.

## Goals and scope

- Preserve the original board, pieces, collision, locking, clearing, and
  scoring behavior.
- Separate game core, input, rendering, persistence, and effects.
- Provide a polished local browser experience without a server or package
  dependency.
- Keep the C++ implementation and tests available for academic comparison.

## Features

- 20 x 15 board and seven tetromino shapes.
- Movement, clockwise rotation, soft drop, hard drop, gravity, locking, line
  clearing, score, level, game over, next preview, pause, and restart.
- Ghost piece, rounded gradient blocks, responsive light theme, line-clear
  particles, and tiered celebration messages.
- Sound effects for movement, rotation, drop, line clear, and game over, with
  a mute button.
- Optional invisible mode hides locked blocks and briefly flashes the board
  when a line is cleared.
- High score and sound preference stored locally in the browser.

Hold, lifetime statistics, settings persistence, production bundling, and
executable packaging are **not implemented**.

## Technology stack

- HTML, CSS, and native JavaScript ES modules.
- Python 3 standard-library HTTP server for local hosting.
- C++17/MSYS2 UCRT64 for the preserved console implementation and tests.
- No npm package, external service, or runtime network request.

## Architecture

`app/game.js` contains `Board`, `Piece`, `Score`, and `Game`. `app/main.js`
maps keyboard/button input to the core, renders DOM state, persists the high
score, and triggers presentation effects. `app/styles.css` owns layout,
themes, block styling, and animation. Details are in
[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) and
[docs/DATA_FLOW.md](docs/DATA_FLOW.md).

## Project structure

```text
app/       Browser application
src/       Preserved modular C++ console implementation
legacy/    Original single-file C++ implementation
tests/     C++ regression/unit tests
docs/      Project, feature, workflow, and report documentation
report/    Existing academic LaTeX report source
```

## Requirements and installation

For the browser app, install a modern browser and Python 3. No package
installation is required. For the preserved C++ build, install an MSYS2
UCRT64 environment with a C++17-capable `g++`.

## Run locally

From the repository root:

```bash
python -m http.server 8000 --directory app
```

Open <http://localhost:8000>. The app is static and does not upload game data.
There is no production build command because the browser files are already
deployable static assets.

## Controls

| Action | Keys |
| --- | --- |
| Move left | `A` / `Left` |
| Move right | `D` / `Right` |
| Soft drop | `S` / `Down` |
| Rotate clockwise | `W` / `Up` |
| Hard drop | `X` / `Space` |
| Pause/resume | `P` or pause button |
| Restart | `Restart` button |

The `◌` button toggles invisible mode. Active and ghost pieces remain visible;
locked blocks are hidden until a line-clear flash.

## Theme, audio, and local data

The current light theme is defined in `app/styles.css`; there is no runtime
theme selector. Local OGG effects are stored in `app/assets/sounds/` and can be
muted with the header button. The high score and mute preference are stored in
browser `localStorage`; board state, settings, and lifetime statistics are not
persisted. See [the audio documentation](docs/features/AUDIO_SYSTEM.md) for
source and license details.

## Build and test the C++ baseline

In an MSYS2 UCRT64 terminal:

```bash
g++ -std=c++17 -Wall -Wextra src/*.cpp -o build/tetris.exe
./build/tetris.exe
```

Build and run focused tests:

```bash
g++ -std=c++17 -Wall -Wextra tests/movement_rotation_tests.cpp \
  src/Board.cpp src/Piece.cpp -o build/movement_rotation_tests.exe
./build/movement_rotation_tests.exe

g++ -std=c++17 -Wall -Wextra tests/score_tests.cpp src/Score.cpp \
  -o build/score_tests.exe
./build/score_tests.exe

g++ -std=c++17 -Wall -Wextra src/Board.cpp src/Piece.cpp src/Renderer.cpp \
  tests/renderer_board_tests.cpp -o build/test_renderer.exe
./build/test_renderer.exe
```

The current environment used for this work did not provide `g++` or `node`;
see [docs/TESTING.md](docs/TESTING.md) for the exact validation boundary.

## Documentation index

- [Project overview](docs/PROJECT_OVERVIEW.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Game design](docs/GAME_DESIGN.md)
- [Legacy behavior](docs/legacy-game-behavior.md)
- [Data flow](docs/DATA_FLOW.md)
- [Feature audit](docs/FEATURE_AUDIT.md)
- [Development workflow](docs/DEVELOPMENT_WORKFLOW.md)
- [Dependencies](docs/DEPENDENCIES.md)
- [Build and deployment](docs/BUILD_AND_DEPLOYMENT.md)
- [Testing](docs/TESTING.md)
- [Troubleshooting](docs/TROUBLESHOOTING.md)
- [Feature documentation](docs/features/)
- [Project reports](docs/reports/)

## Git workflow

Use focused Conventional Commits (`feat`, `fix`, `docs`, `test`, `refactor`,
`chore`). Inspect status before staging, stage only related files, validate
before committing, and never force-push or rewrite existing history. The
conflict lab and resolution are documented in
[docs/reports/CONFLICT_RESOLUTION_REPORT.md](docs/reports/CONFLICT_RESOLUTION_REPORT.md).

## Known limitations and future improvements

The C++ tests are not integrated into a CI runner, and the browser has no
automated UI test suite. Future work may add hold, audio, theme settings,
lifetime statistics, seeded replay tests, and browser automation.

No license has been declared for this educational repository.
