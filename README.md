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

Configure and build the project:

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## Run

Run `compositor`:

```shell
./compositor
```

Any client app can now be started, in this case we run `webbrowser`.
The build configures `webbrowser` with an RPATH that locates `librevyv`,
so no additional `LD_LIBRARY_PATH` setup is required:

```shell
./webbrowser --frame="200,200,1000,600" --url="https://youtube.com"
./webbrowser --frame="400,50,1000,600" --url="https://www.waze.com/live-map"
./webbrowser --frame="800,100,1000,600" --url="https://google.com"
```

## Licensing

Revyv is released under the [The MIT License](./LICENSE).
