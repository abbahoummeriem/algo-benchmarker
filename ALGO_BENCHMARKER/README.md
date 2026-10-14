# ⚡ Mini Algo Benchmarker

A lightweight, zero-bloat sorting algorithm benchmarker and Gnuplot visualization tool written in pure **ISO C11**.

Designed to benchmark the 7 classic sorting algorithms on arrays up to **1,000,000** elements and generate a high-resolution, dual-panel comparison chart.

---

## 🎯 Features

- **Single Purpose & Zero Bloat**: No complex CLI menus. Just run it or override via environment variables.
- **Pure Native C11**: Directly sorts contiguous memory arrays with zero abstraction overhead.
- **Accurate Time-Wise**:
  - High-resolution monotonic timers: `CLOCK_MONOTONIC_RAW` on Linux, `QueryPerformanceCounter` on Windows.
  - Multi-iteration runs to filter out OS scheduling jitter.
  - CPU cache & frequency governor warm-up pass before measurement.
  - Algorithmic correctness validation (is-sorted check + 64-bit sum and XOR checksum invariance).
- **Scales up to 1,000,000 Elements**:
  - Fast $O(N \log N)$ and $O(N^{1.3})$ algorithms (Quick, Merge, Heap, Shell) sort 1M elements in under a third of a second.
  - Quadratic $O(N^2)$ algorithms (Bubble, Selection, Insertion) are benchmarked up to $N = 100,000$, and intelligently skipped beyond to keep the test runtime snappy (~30-40 seconds).
  - Can force exhaustive execution on all sizes using `BENCH_ALL=1`.
- **Gnuplot Integration (Dual-Panel 4K UHD)**:
  - Direct inline dataset rendering — no intermediate `.dat` files created or left behind.
  - Generates 4K UHD (3840x2160) PNG (`plots/benchmark.png`) with side-by-side analysis:
    1. **Log-Log Complexity Curve**: Visualizes asymptotic complexity slopes ($O(N^2)$ vs $O(N \log N)$).
    2. **Linear Scalability Curve**: Direct comparison of high-performance algorithms up to 1M elements.
- **Cross-Platform Auto-Viewer**:
  - Automatically opens the rendered chart in your OS default image viewer (`xdg-open` / `gio open` / `open` / `ShellExecute`).

---

## 🚀 Usage

### 1. Build
```bash
make
```

### 2. Configuration & Execution Modes

You can toggle between **Fast Skip Mode** (default) and **Exhaustive 1M Mode** using the `#define` constant in [main.c](file:///home/8l4z3/Tmp/algo_benchmarker/src/main.c):

```c
/* In src/main.c */
#define BENCHMARK_RUN_ALL 0   /* 0 (or false): skip O(N²) for N > 100k (~35 sec total) */
                              /* 1 (or true) : run ALL tests even 1M for all algos */
```

- **Run default (Fast Skip Mode)**:
```bash
make
./bin/algo_benchmarker
```

- **Run exhaustive mode (either by editing `main.c` or via CLI environment variable)**:
```bash
BENCH_ALL=1 ./bin/algo_benchmarker
```

---

## 🧬 Benchmarked Algorithms

| ID | Algorithm | Average Time | Worst Time | Space | Note |
| :--- | :--- | :--- | :--- | :--- | :--- |
| `quick` | Quick Sort | $O(N \log N)$ | $O(N^2)$ | $O(\log N)$ | Median-of-3 + insertion cutoff |
| `merge` | Merge Sort | $O(N \log N)$ | $O(N \log N)$ | $O(N)$ | Pre-allocated aux buffer |
| `heap` | Heap Sort | $O(N \log N)$ | $O(N \log N)$ | $O(1)$ | In-place binary max heap |
| `shell` | Shell Sort | $O(N^{1.3})$ | $O(N^{1.5})$ | $O(1)$ | Knuth ($3x+1$) gap sequence |
| `insertion` | Insertion Sort | $O(N^2)$ | $O(N^2)$ | $O(1)$ | Cache-friendly quadratic sort |
| `selection` | Selection Sort | $O(N^2)$ | $O(N^2)$ | $O(1)$ | Minimum swap count |
| `bubble` | Bubble Sort | $O(N^2)$ | $O(N^2)$ | $O(1)$ | Early-exit flag |

---

## 📊 Output Artifacts

- **Terminal**: Real-time progress table displaying high-precision timings across all tested sizes ($N = 1,000$ to $1,000,000$).
- **Chart** (Saved in `plots/`):
  - `plots/benchmark.png`: High-resolution linear comparison chart of **All 7 Algorithms**.
    - **In Fast Skip Mode (`BENCHMARK_RUN_ALL 0`)**: Displays all 7 algorithms up to $N = 100,000$ with complete data.
    - **In Exhaustive Mode (`BENCHMARK_RUN_ALL 1`)**: Displays all 7 algorithms across the full test range up to $N = 1,000,000$.

---

## 🔬 Theoretical Interpretation

1. **Quadratic Explosion ($O(N^2)$)**:
   - For $N = 100,000$, Bubble Sort takes ~25-30 seconds ($5 \times 10^9$ operations).
   - Scaling to $N = 1,000,000$ ($10\times$ increase) multiplies quadratic operations by $100\times$, requiring over **45 minutes** for a single bubble sort pass.
   - On the log-log chart, this is evidenced by a steep slope of $k = 2$.

2. **Log-Linear Efficiency ($O(N \log N)$ & $O(N^{1.3})$)**:
   - Quick Sort, Merge Sort, Heap Sort, and Shell Sort scale effortlessly to 1M elements, taking between **0.10s and 0.30s**.
   - Quick Sort exhibits the best cache locality and instruction pipelining, followed closely by Merge Sort, Heap Sort, and Shell Sort.
