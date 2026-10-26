# Algorithm notes and review

## 0/1 Knapsack

For each item `(w_i, v_i)` choose `x_i` in `{0, 1}` to maximize `sum(v_i x_i)`, subject to `sum(w_i x_i) <= W`.

## Value-scaling FPTAS

Ignore non-positive-value items and individual items heavier than capacity. For the remaining `n` items, set `Vmax` to their largest original profit and `K = epsilon * Vmax / n`. Scaled profits `q_i = floor(v_i/K)` are integers. The recurrence for minimum weight to achieve scaled profit `s` is:

`D_i[s] = min(D_(i-1)[s], w_i + D_(i-1)[s-q_i])`.

One-dimensional descending iteration enforces the 0/1 constraint. The implementation also tracks the real profit of the witness associated with each state and returns the maximum real profit among feasible states. This avoids reporting the scaled-back proxy as the objective.

The rounding loss of any feasible solution is less than `nK = epsilon*Vmax`, and `Vmax <= OPT` because the item attaining `Vmax` is eligible. Thus `profit >= OPT - epsilon*Vmax >= (1-epsilon)*OPT` for a maximizing scaled solution. Tie selection by actual profit does not worsen that guarantee.

The bound assumes ideal arithmetic; the code uses `double` scaling and has a finite 4-million-state safeguard. Very small epsilon can trigger the safeguard.

## Exact dynamic programming

Let `DP[w]` be the best attainable original profit within weight `w`. For each item, iterate `w` descending and update `DP[w] = max(DP[w], DP[w-weight_i]+value_i)`. This is exact for integer weights with time `O(nW)` and memory `O(W)`, practical for the provided capacities.

## Changes versus the original

1. Corrects the reported FPTAS output: `best_scaled_value*K` was **not necessarily an achievable original profit**.
2. Validates files, item data, epsilon, and capacity; excludes infeasible and zero-value items.
3. Adds a capacity-based exact solver and reports which algorithm is used, rather than silently labeling an exact result FPTAS.
4. Avoids unbounded scaled-DP allocation, uses 64-bit accumulated profits and weights, and supports CLI arguments.
5. Tests the implementation on small instances versus brute-force search.

## Limitations

This implementation returns objective values, not the selected item indices. The FPTAS uses dense scaled-profit states and may be impractical for very large inputs/small epsilon, where sparse state pruning or more sophisticated schemes may be warranted. The `auto` method's simple threshold is a pragmatic heuristic, not an algorithmically optimal selector.
