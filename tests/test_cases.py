"""Regression/property tests. Run: python tests/test_cases.py"""
import itertools
import random
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
BIN = ROOT / 'knapsack_test_bin'
subprocess.run(['g++', '-std=c++17', '-O2', '-Wall', '-Wextra', str(ROOT/'src/main.cpp'), '-o', str(BIN)], check=True)

def run(items, cap, eps, method):
    with tempfile.TemporaryDirectory() as d:
        p = Path(d) / 'items.txt'
        p.write_text(''.join(f'{w} {v}\n' for w,v in items))
        out = subprocess.run([str(BIN), str(p), str(cap), str(eps), '--method', method], check=True, text=True, capture_output=True).stdout
        return int(next(line.split(':')[1] for line in out.splitlines() if line.startswith('Objective value:')))

def brute(a, W):
    return max((sum(v for (w,v), use in zip(a,mask) if use)
                for mask in itertools.product((0,1), repeat=len(a))
                if sum(w for (w,v),use in zip(a,mask) if use) <= W), default=0)

rng = random.Random(42)
for i in range(75):
    items = [(rng.randint(1,20), rng.randint(0,50)) for _ in range(rng.randint(0,10))]
    capacity = rng.randint(0,50)
    optimum = brute(items, capacity)
    exact = run(items,capacity,0.3,'exact')
    approx = run(items,capacity,0.3,'fptas')
    assert exact == optimum, (items,capacity,exact,optimum)
    assert (1-0.3)*optimum - 1e-8 <= approx <= optimum, (items,capacity,approx,optimum)
print('PASS: 75 deterministic randomized tests, exact and FPTAS guarantee')
BIN.unlink()
