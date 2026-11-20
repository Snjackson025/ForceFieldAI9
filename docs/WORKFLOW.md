# ForcefieldAi9 Workflow

## What it does
On every push or pull request to `main`/`master` touching `forcefield.cpp`,
the check script, or the workflow file, GitHub Actions:

1. Checks out the code (`actions/checkout@v4`).
2. Compiles with `g++ -std=c++17 -Wall -Wextra -o forcefield_test forcefield.cpp`.
3. Runs the 5D simulation smoke test and verifies the output contains
   `N-DIMENSIONAL FORCE FIELD SYSTEM v2.0`, `Particles: 3`, and
   `Simulation complete. Final time: 1.00`.

The workflow fails if the code does not compile or the simulation output
does not match — no always-pass steps.

## Local check
Run `./scripts/forcefield-check.sh` — it does the same compile + smoke test.

## Adding the workflow
Muse cannot push `.github/workflows/*` files through the GitHub API, so the
workflow file must be added manually in the GitHub web UI at
`.github/workflows/forcefield.yml`, committed directly to the default branch.
