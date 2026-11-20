#!/bin/bash
# ForcefieldAi9 local check: compile + run simulation smoke test
set -e
cd "$(dirname "$0")/.."
echo "Compiling forcefield.cpp..."
g++ -std=c++17 -Wall -Wextra -o forcefield_test forcefield.cpp
echo "Running simulation..."
./forcefield_test > run.log
grep -q "N-DIMENSIONAL FORCE FIELD SYSTEM v2.0" run.log
grep -q "Simulation complete. Final time: 1.00" run.log
grep -q "Particles: 3" run.log
echo "All ForcefieldAi9 checks passed."
