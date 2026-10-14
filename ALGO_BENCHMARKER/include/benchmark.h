#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stdbool.h>

/*
 * ============================================================================
 * Global Configuration Toggle:
 * - Set to 1 (or true) : Run ALL tests up to 1,000,000 elements for ALL algorithms
 *                        (Warning: O(N^2) quadratic sorts like Bubble take ~45 min at 1M).
 * - Set to 0 (or false): Use skip method for O(N^2) quadratic algorithms for N > 100,000
 *                        (Fast mode: completes in ~30-35 seconds).
 * ============================================================================
 */
#ifndef BENCHMARK_RUN_ALL
#define BENCHMARK_RUN_ALL 0
#endif

/**
 * Runs the full sorting algorithm benchmark suite.
 *
 * Benchmarks all registered algorithms across array sizes from 1,000
 * to 1,000,000 elements, prints a formatted timing table to stdout,
 * and generates a Gnuplot chart (PNG) in the plots/ directory.
 *
 * When @p run_all is false, O(N^2) algorithms (Insertion, Selection,
 * Bubble) are skipped for sizes above 100,000 to avoid prohibitive
 * runtimes.  Can also be overridden at runtime via BENCH_ALL=1 env var.
 *
 * @param run_all  If true, run every algorithm at every size including 1M.
 *                 If false, skip quadratic algorithms for N > 100,000.
 * @return         Always returns 0.
 */
int run_benchmark_custom(bool run_all);

/**
 * Convenience macro that calls run_benchmark_custom() with the
 * compile-time BENCHMARK_RUN_ALL setting.
 *
 * Usage: return run_benchmark();
 */
#define run_benchmark() run_benchmark_custom((BENCHMARK_RUN_ALL) != 0)

#endif /* BENCHMARK_H */
