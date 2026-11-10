# Dependencies

## Browser runtime

No package or third-party runtime dependency is used. The browser provides
ES modules, DOM APIs, `requestAnimationFrame`, and `localStorage`.

## Development/build tools

- Python 3 is optional and used only as a static local HTTP server.
- A modern browser is required to run the web app.
- MSYS2 UCRT64 `g++` with C++17 support is required only for the preserved
  C++ executable and tests.

The app includes five locally stored OGG sound effects from the Kenney
Interface Sounds set, downloaded through the
`hipdiscovery/Free-Sound-Effects` catalog. The files and the source license are
kept in `app/assets/sounds/`; the catalog repository is MIT-licensed and the
original Kenney pack is documented as CC0.

There is no package manifest or lockfile, so no package versions or install
commands are claimed. No unused package was found to remove.
