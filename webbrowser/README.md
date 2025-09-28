# Webbrowser

The `webbrowser` target integrates the Chromium Embedded Framework (CEF) into
Revyv. The build automatically downloads the matching CEF binary distribution
and links it against the project.

## CEF version

The project currently targets **CEF 140.1.14+geb1c06e+chromium-140.0.7339.185**
(Chromium 140.0.7339.185). The version is configured in
[`CMakeLists.txt`](CMakeLists.txt) and used when downloading the binary release.

## Building

To configure and build the target run:

```bash
cmake -S webbrowser -B out/webbrowser
cmake --build out/webbrowser
```

The configure step downloads approximately 250 MB of prebuilt CEF binaries, so
the initial run can take a few minutes depending on network speed.

## Inspecting the CEF source code

CEF's binary distribution ships with headers and samples, but it can be helpful
to inspect the upstream source code when adapting the integration to a new
release. The helper script below clones the CEF repository at the same tag as
the binary build and places it under `webbrowser/third_party/cef_source`:

```bash
cd webbrowser
./tools/fetch_cef_source.py
```

Use the `--version`, `--repo`, or `--output-dir` flags to target a different tag
or repository mirror. Once cloned you can search the upstream implementation for
API changes that may affect the Revyv integration.
