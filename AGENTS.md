# Agent Instructions

## Scope
These instructions apply to the entire repository.

## Project layout
- `compositor/`: Wayland-like compositor implemented with SDL2, OpenGL, ZeroMQ, and LZO.
- `librevyv/`: Shared library exposing a C ABI used by clients to talk to the compositor. Contains a small `client-test` executable.
- `webbrowser/`: Chromium Embedded Framework (CEF) based browser client. Configuring this target downloads a multi-hundred-megabyte CEF bundle.

## Prerequisites (Debian/Ubuntu)
Install build tools and system packages before configuring CMake:

```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential cmake ninja-build \
  libsdl2-dev libzmq3-dev libczmq-dev liblzo2-dev \
  libcairo2-dev libglu1-mesa-dev libx11-dev
```

## Configure
The project uses out-of-tree builds. Create a build directory once you have the dependencies:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
```

> **Tip:** If you only need the compositor and library, configure them as standalone projects to avoid the large CEF download triggered by `webbrowser/`:
>
> ```bash
> cmake -S compositor -B build/compositor -DCMAKE_BUILD_TYPE=Release
> cmake -S librevyv -B build/librevyv -DCMAKE_BUILD_TYPE=Release
> ```

## Build
To build everything that was configured:

```bash
cmake --build build --parallel
```

Common focused builds:

```bash
cmake --build build/compositor --parallel
cmake --build build/librevyv --target revyv --parallel
cmake --build build/librevyv --target client-test --parallel
```

## Tests
The only automated check in-tree today is the `client-test` executable from `librevyv/test`. After building it, run:

```bash
./build/librevyv/client-test
```

## Runtime quickstart
1. Start the compositor from its build output directory: `./build/compositor/compositor`.
2. Add the built library to `LD_LIBRARY_PATH`: `export LD_LIBRARY_PATH="$(pwd)/build/librevyv"`.
3. Launch a client (for example `webbrowser`) from a separate terminal.

## Coding conventions
- Follow the existing `.clang-format` file when touching C++ sources.
- Do not add `try`/`catch` blocks around include directives or imports.
- Prefer CMake out-of-tree builds; do not write build artifacts into source directories.
