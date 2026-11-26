# ForceFieldAI9

**Advanced N-Dimensional Force Field System v2.0.**

## What it does

Simulates particles moving through layered force fields in N dimensions:

- **VecN** — N-dimensional vectors
- **HyperSphericalField** — four falloff types
- **ReflectiveField** — damping fields
- **TensorField** — anisotropic fields
- **CoupledField** — dimensional coupling
- **VortexField, DragField, OscillatingField** — motion fields
- **FieldStack** — spatial hashing over stacked fields

The demo runs 3 particles in 5 dimensions through 5 fields for 1000
steps, then prints the net force. The original v1 files sit alongside
v2.0 in the same repo.

## Build and run

Requires C++17. With Visual Studio, open the solution and build
Debug x64. Or with g++:

```sh
g++ -std=c++17 -Wall -Wextra -o forcefield forcefield.cpp
./forcefield
```

## Continuous integration

`.github/workflows/forcefield.yml` (v2.0) and
`.github/workflows/build-and-test.yml` (v1) build Debug x64 with MSBuild
and run the programs, checking their output. Both also run daily at
06:00 UTC.
