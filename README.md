# 0/1 Knapsack — FPTAS and Exact Dynamic Programming (C++17)

A reproducible C++ implementation of the **0/1 Knapsack Problem**, including a **profit-scaling fully polynomial-time approximation scheme (FPTAS)** and an **exact weight-indexed dynamic program**. The `auto` mode uses exact DP when the capacity is small, making the provided benchmark cases efficient.

## Problem

Choose items to maximize total profit, subject to a weight budget:

```text
maximize   sum(value[i] * x[i])
subject to sum(weight[i] * x[i]) <= capacity
           x[i] in {0, 1}
```

## Algorithms

- **Value-scaling FPTAS:** let `K = epsilon * Vmax / n`, where `n` is the count of *eligible positive-value items* and `Vmax` is their maximum value. Round each profit down to `floor(value[i]/K)`; DP tracks minimum weight for every scaled profit. Returns the **real profit of a feasible DP witness**, not `scaled_profit * K`. For `0 < epsilon < 1`, its profit is at least `(1-epsilon) OPT`, subject to normal floating-point arithmetic assumptions. Its classical time bound is `O(n^3 / epsilon)` and it can use `O(n^2 / epsilon)` states.
- **Exact DP by weight:** for a reasonably small integer capacity `W`, calculate the optimal profit directly in `O(nW)` time and `O(W)` memory.
- **Auto:** chooses exact DP when `W <= 100000`; otherwise chooses value-scaling FPTAS. A guard limits the FPTAS array to 4 million states; if exceeded, it reports an error instead of exhausting memory. This is a deliberate implementation limit, **not** a claim that every instance will fit.

All items must have positive integer weights and nonnegative integer profits. Item order is arbitrary.

## Build

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic src/main.cpp -o knapsack
```

Windows (Visual Studio Developer PowerShell):

```powershell
cl /std:c++17 /O2 /EHsc src\main.cpp /Fe:knapsack.exe
```

## Run

```bash
./knapsack data/items_100.txt 800 0.95 --method fptas
./knapsack data/items_1000.txt 1500 0.95 --method auto
./knapsack data/items_8000.txt 3000 0.95 --method exact
```

Windows: use `./knapsack.exe` (PowerShell also accepts `.\knapsack.exe`). Arguments: `path capacity [epsilon=0.1] [--method auto|exact|fptas]`. Epsilon must satisfy `0 < epsilon < 1`.

## Supplied datasets

| Dataset | Items | Capacity | Verified optimal objective (exact DP) |
|---|---:|---:|---:|
| `items_100.txt` | 100 | 800 | 8529 |
| `items_1000.txt` | 1000 | 1500 | 32178 |
| `items_8000.txt` | 8000 | 3000 | 79866 |

These values were calculated with the included exact dynamic program, compiled with GCC 14.2.0. **The values are exact for the specified capacities; timings are machine-dependent.** In `auto` mode, all three datasets use the exact algorithm, so they are **not** benchmarks of the FPTAS itself. On the 100-item dataset at `epsilon=0.95`, the FPTAS also returned 8529 in our run.

## Tests

```bash
python tests/test_cases.py
```

The test script compiles the implementation and checks 75 seeded small random instances against exhaustive search, including the `(1-epsilon)` approximation condition. Requires Python 3 and a C++17 `g++` compiler.

## Repository layout

- `src/main.cpp` — implementation and command-line interface
- `data/` — original supplied test datasets
- `tests/test_cases.py` — repeatable correctness tests
- `docs/algorithm.md` — algorithm discussion and implementation constraints
- `.gitignore` — excludes IDE and build artifacts

## Notes

This project began as a Visual Studio experiment. The old `.vcxproj.user` file is intentionally omitted because it contains machine-specific IDE settings. The code is now standalone, portable C++17. A state-limit error in `--method fptas` means the experiment requires a different capacity/epsilon combination or a more advanced approximation implementation.
