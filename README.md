# Revyv

Compositing display server and web browser client. The compositor uses OpenGL textures for window compositing, ZeroMQ for IPC and SDL for input handling. The web browser client is based on CEF (Chromium Embedded Framework).

**compositor**

Written in C++17, uses OpenGL for hardware accelerated rendering, Linux shared memory and ZeroMQ for inter-process communication, and SDL for mouse and keyboard event handling.

**librevyv**

Low-level C API used by client apps to communicate with `compositor`, it is implemented in C++ but it has a C ABI that makes it easier to port to other programming languages.

**webbrowser**

A web browser based on Chromium Embedded Framework (CEF), uses `librevyv` to communicate with `compositor`.

## Build

#### Ubuntu

_(Tested on Ubuntu 22.10 Kinetic Kudu)_

Install CMake:

```shell
sudo apt install cmake
```

Install dependencies:

```
sudo apt install libsdl2-dev libcairo-dev libzmq3-dev cppzmq-dev liblzo2-dev
```

#### openSUSE

_(Tested on openSUSE Leap 15.4)_

Install CMake:

```shell
sudo zypper install cmake
```

Install dependencies:

```
sudo zypper install SDL2-devel cairo-devel zeromq-devel cppzmq-devel lzo-devel
```

## Run

Run `compositor`:

```shell
./compositor
```

Add `librevyv` to `LD_LIBRARY_PATH`:

```shell
export LD_LIBRARY_PATH=/path/to/cmake/build/librevyv
```

Any client app can now be started, in this case we run `webbrowser`:

```shell
./webbrowser --frame="200,200,1000,600" --url="https://youtube.com"
./webbrowser --frame="600,400,1000,600" --url="https://dw.com"
./webbrowser --frame="400,50,1000,600" --url="https://www.waze.com/live-map"
./webbrowser --frame="800,100,1000,600" --url="https://google.com"
```

## Licensing

Revyv is released under the [The MIT License](./LICENSE).
