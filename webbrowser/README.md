# Webbrowser

The `webbrowser` target integrates the Chromium Embedded Framework (CEF) into
Revyv. The build automatically downloads the matching minimal CEF binary
distribution and links it against the project.

## CEF version

The project currently targets **CEF 140.1.14+geb1c06e+chromium-140.0.7339.185**
(Chromium 140.0.7339.185). The version is configured in
[`CMakeLists.txt`](CMakeLists.txt) and used when downloading the binary release.

## Prerequisites

Install the development packages required by CEF and the Revyv integration.
The list below covers the libraries used by the default configuration on
Linux:

```bash
sudo apt-get install \
  libcairo2-dev libzmq3-dev liblzo2-dev \
  libatk1.0-dev libatk-bridge2.0-dev libatspi2.0-dev \
  libcups2-dev libxcomposite-dev libxdamage-dev libxfixes-dev \
  libxrandr-dev libgbm-dev libxkbcommon-dev libasound2-dev
```

## Building

To configure and build the target run:

```bash
cmake -S webbrowser -B out/webbrowser
cmake --build out/webbrowser
```

The configure step downloads the minimal prebuilt CEF binaries (roughly
120 MB), so the initial run can take a few minutes depending on network speed.

`cmake --build` will also compile the `librevyv` shared library and its
`client-test` executable when the `webbrowser` project is configured on its own.

## Inspecting the CEF source code

CEF's binary distribution ships with headers and samples, but it can be helpful
to inspect the upstream source code when adapting the integration to a new
release. Clone the matching revision of the CEF repository in a separate
workspace to reference implementation details and API usage patterns as you
update the integration.
