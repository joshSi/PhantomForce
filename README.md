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

Move with `WASD`, aim with the mouse, `Esc` pauses and `Space` toggles the
collision debug overlay (collision shapes and contact normals).

## Test

The physics engine has a unit test suite that runs without a window:

```
cmake --build build
ctest --test-dir build --output-on-failure
```

Pass `-DPHANTOMFORCE_BUILD_TESTS=OFF` when configuring to skip building it.

## Physics

Gameplay objects are simulated by a small 2D rigid-body engine in
`include/physics/`:

- `physics::Body` – a circle or (rotatable) box with mass, velocity,
  restitution (bounciness), friction, linear damping and a force accumulator,
  plus angular velocity, moment of inertia, torque and angular damping.
  A mass of `0` makes a body static; a static body with a velocity acts as a
  kinematic mover. `setFixedRotation(true)` keeps collisions from spinning a
  body (the player uses this so it can keep facing the mouse).
- `physics::PhysicsWorld` – holds bodies (not owned) and advances them with
  `step(dt)`: a sort-and-sweep broad phase on bounding boxes, exact
  circle/box narrow-phase tests (separating-axis test with face clipping for
  rotated boxes) that produce contact points, iterative impulse resolution
  at those points with restitution and Coulomb friction, then positional
  correction so bodies do not sink into each other. Hits away from the centre
  of mass spin bodies, friction makes balls roll, and linear and angular
  momentum are conserved. Optional gravity and a per-contact callback are
  available for game logic.
- `Object` (and its `Circle` / `Rectangle` subclasses) is an `sf::Sprite`
  that is also a `physics::Body`; the sprite's position and rotation are the
  body's position and angle. `Player` is a dynamic `Circle` that turns input
  into acceleration.

Positions are in pixels, angles in radians (clockwise on screen) and time in
seconds. The game steps the world on a fixed 120 Hz timestep independent of
the render frame rate.

```cpp
physics::PhysicsWorld world;          // no gravity: top-down game
Rectangle* wall = new Rectangle(tex, {40.f, 400.f});   // mass 0 => static
Circle* ball = new Circle(tex, 20.f);
ball->setMass(3.f);
ball->setRestitution(0.6f);
world.addBody(wall);
world.addBody(ball);
world.step(1.f / 120.f);
```

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md)
