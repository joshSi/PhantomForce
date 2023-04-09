# Phantom Force

A game built with the [SFML library](https://www.sfml-dev.org)

## Build

For a single-configuration generator (typically the case on Linux and macOS):

```
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

For a multi-configuration generator (typically the case on Windows):

```
cmake -S . -B build
cmake --build build --config Release
```
