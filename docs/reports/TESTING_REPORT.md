# Testing report

## Environment

The browser smoke test used the local Python static server and a browser
automation session. The environment did not expose `g++` or `node`.

## Results

- Browser page load: **PASS**.
- Board render: **PASS** — 300 cells.
- Next preview render: **PASS** — 16 cells.
- Keyboard smoke input: **PASS** — A/W/X accepted.
- Formatting check: **PASS** — `git diff --check`.
- C++ movement/rotation tests: **BLOCKED** — `g++` unavailable.
- C++ score tests: **BLOCKED** — `g++` unavailable.
- C++ renderer tests: **BLOCKED** — `g++` unavailable.
- JavaScript CLI syntax check: **BLOCKED** — `node` unavailable.
- Automated line-clear/persistence browser suite: **NOT IMPLEMENTED**.

No unverified test is reported as passed.
