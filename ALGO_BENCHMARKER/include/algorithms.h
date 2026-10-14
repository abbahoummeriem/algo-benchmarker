#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <stdbool.h>

/**
 * Metadata descriptor for a registered sorting algorithm.
 *
 * Each entry in the internal registry maps a human-readable name and
 * short identifier to a concrete sort function, along with a flag
 * indicating its asymptotic complexity class.
 */
typedef struct {
    const char *id;                     /**< Short identifier (e.g. "quick", "bubble"). */
    const char *name;                   /**< Human-readable display name (e.g. "Quick Sort"). */
    void (*func)(int *arr, int n);      /**< Sorting function pointer. Sorts arr[0..n-1] in-place, ascending. */
    bool is_quadratic;                  /**< True if worst-case is O(N^2); used to skip large benchmarks. */
} AlgorithmDef;

/**
 * Returns the total number of registered sorting algorithms.
 *
 * @return  Algorithm count (currently 7). Always >= 0.
 */
int get_algorithm_count(void);

/**
 * Retrieves a registered algorithm descriptor by index.
 *
 * @param index  Zero-based index into the internal registry.
 *               Must satisfy 0 <= index < get_algorithm_count().
 * @return       Pointer to a static AlgorithmDef, or NULL if index is out of range.
 */
const AlgorithmDef *get_algorithm(int index);

/**
 * Fills an array with deterministic pseudo-random integers in [0, 9999999].
 *
 * Uses a fast 32-bit Xorshift PRNG seeded by @p seed.  The same seed
 * always produces the same sequence, which is essential for benchmark
 * reproducibility and correctness verification.
 *
 * @param arr   Output array. Must have room for at least @p n elements.
 *              No-op if NULL.
 * @param n     Number of elements to generate. No-op if <= 0.
 * @param seed  PRNG seed. If 0, a built-in default seed (123456789) is used.
 */
void generate_random_data(int *arr, int n, unsigned int seed);

/**
 * Checks whether an integer array is sorted in non-decreasing order.
 *
 * @param arr  Array to check. Returns true if NULL.
 * @param n    Number of elements. Returns true if n <= 1.
 * @return     true if arr[i] <= arr[i+1] for all valid i, false otherwise.
 */
bool is_sorted(const int *arr, int n);

/**
 * Computes additive and XOR checksums over an integer array.
 *
 * Used to verify that a sorting algorithm is a permutation of the input
 * (i.e. no elements were lost, duplicated, or corrupted).
 *
 * @param arr      Input array of @p n elements.
 * @param n        Number of elements.
 * @param sum      Output: sum of all elements. May be NULL if not needed.
 * @param xor_val  Output: XOR of all elements. May be NULL if not needed.
 */
void compute_checksums(const int *arr, int n, long long *sum, long long *xor_val);

#endif /* ALGORITHMS_H */
