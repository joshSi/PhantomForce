# Phantom Force

A game built with the [SFML library](https://www.sfml-dev.org)

[![CI](https://github.com/joshSi/PhantomForce/actions/workflows/ci.yml/badge.svg?branch=main)](https://github.com/joshSi/PhantomForce/actions/workflows/ci.yml)

## Build

For Linux and Mac, the CMake config does not automatically install sfml and assumes the libraries already exist on the system. See below to install sfml:

### Prerequisites

#### Linux

```sh
# May need to run to fetch latest packages
sudo apt-get update
sudo apt-get install libsfml-dev
```

#### MacOS

```sh
# May need to run to fetch latest packages
brew update
brew install sfml
```

#### Windows

Windows does not need to install sfml as a prerequisite like Linux and MacOS do. Instead, the CMake config will download sfml using the FetchContent module.

https://github.com/joshSi/PhantomForce/blob/532d64e13dc105588dd303b4c299d734e714c68e/CMakeLists.txt#L17-L25

### CMake

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

## Run

```
cd build
./PhantomForce
```

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md)
