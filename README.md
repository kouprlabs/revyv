# Revyv

Compositing display server and web browser client. The compositor uses OpenGL textures for window compositing, ZeroMQ for IPC and SDL for input handling. The web browser client is based on CEF (Chromium Embedded Framework).

**compositor**

Written in C++17, uses OpenGL for hardware accelerated rendering, Linux shared memory and ZeroMQ for inter-process communication, and SDL for mouse and keyboard event handling.

**librevyv**

Low-level C API used by client apps to communicate with `compositor`, it is implemented in C++ but it has a C ABI that makes it easier to port to other programming languages.

**webbrowser**

A web browser based on Chromium Embedded Framework (CEF), uses `librevyv` to communicate with `compositor`.

## Build

#### Fedora

Install CMake:

```shell
sudo dnf install cmake
```

Install dependencies:

```
sudo dnf install SDL2-devel cairo-devel zeromq-devel cppzmq-devel lzo-devel
```

#### macOS

Install the required toolchain and libraries with [Homebrew](https://brew.sh/):

```shell
brew install cmake ninja sdl2 zeromq cppzmq lzo cairo
```

Configure and build the project:

```
cmake -S . -B build
cmake --build build
```

## Run

Run `compositor`:

```shell
./compositor --width=1440 --height=900
```

Any client app can now be started, in this case we run `webbrowser`.

```shell
./webbrowser --frame="0,0,1440,900" --url="https://youtube.com"
./webbrowser --frame="0,0,1440,900" --url="https://www.waze.com/live-map"
./webbrowser --frame="0,0,1440,900" --url="https://google.com"
```

## Licensing

Revyv is released under the [The MIT License](./LICENSE).
