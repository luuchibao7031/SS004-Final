# Build and deployment

## Browser app

The browser app has no compilation or production bundle. Run it from the
repository root:

```text
python -m http.server 8000 --directory app
```

Then open `http://localhost:8000`. Any equivalent static server can serve
`app/`; do not open the HTML via a `file://` URL because module loading can be
restricted.

## Preserved C++ build

In an MSYS2 UCRT64 terminal:

```text
g++ -std=c++17 -Wall -Wextra src/*.cpp -o build/tetris.exe
```

The generated executable is ignored by Git. Tests are compiled individually;
commands are listed in [TESTING.md](TESTING.md). No `.exe` packaging is
provided for the browser version.

## Verified boundaries

The local server and browser page were smoke-tested. This environment did not
provide `g++` or `node`, so C++ compilation and CLI JavaScript checks remain
unverified here.
