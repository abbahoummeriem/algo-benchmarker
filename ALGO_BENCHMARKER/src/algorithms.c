#include "algorithms.h"
#include <stdlib.h>
#include <string.h>

/* ========================================================================= */
/* Pseudo-Random Generator (Fast & Deterministic 32-bit Xorshift)            */
/* ========================================================================= */

/**
 * Advances a 32-bit Xorshift PRNG state and returns the next value.
 *
 * @param state  Pointer to the current PRNG state (modified in-place).
 * @return       Next pseudo-random 32-bit unsigned integer.
 */
static inline unsigned int prng_next(unsigned int *state) {
    unsigned int x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

void generate_random_data(int *arr, int n, unsigned int seed) {
    if (!arr || n <= 0) return;
    unsigned int rng = seed ? seed : 123456789;
    for (int i = 0; i < n; i++) {
        arr[i] = (int)(prng_next(&rng) & 0x7FFFFFFF) % 10000000;
    }
}

bool is_sorted(const int *arr, int n) {
    if (!arr || n <= 1) return true;
    for (int i = 0; i < n - 1; i++) {
        if (arr[i] > arr[i + 1]) {
            return false;
        }
    }
    return true;
}

void compute_checksums(const int *arr, int n, long long *sum, long long *xor_val) {
    long long s = 0;
    long long x = 0;
    for (int i = 0; i < n; i++) {
        s += arr[i];
        x ^= (long long)arr[i];
    }
    if (sum) *sum = s;
    if (xor_val) *xor_val = x;
}

/* ========================================================================= */
/* Sorting Algorithm Implementations                                         */
/* ========================================================================= */

/**
 * Swaps two integers in-place via a temporary variable.
 *
 * @param a  Pointer to the first integer.
 * @param b  Pointer to the second integer.
 */
static inline void swap_int(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

/**
 * Insertion Sort — O(N^2) average/worst, O(N) best (nearly sorted).
 *
 * Stable, in-place. Used as the base-case fallback inside Quick Sort
 * and Merge Sort for small sub-arrays (N < 16).
 *
 * @param arr  Array to sort in-place, ascending. No-op if NULL.
 * @param n    Number of elements. No-op if n <= 1.
 */
static void algo_insertion_sort(int *arr, int n) {
    if (!arr || n <= 1) return;
    for (int i = 1; i < n; i++) {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

/**
 * Selects a pivot via median-of-three and partially sorts the
 * low/mid/high elements as a side-effect.
 *
 * @param arr   Array being partitioned.
 * @param low   Index of the first element in the range.
 * @param high  Index of the last element in the range.
 * @return      Index of the median element (used as pivot).
 */
static inline int median_of_three(int *arr, int low, int high) {
    int mid = low + (high - low) / 2;
    if (arr[low] > arr[mid]) swap_int(&arr[low], &arr[mid]);
    if (arr[low] > arr[high]) swap_int(&arr[low], &arr[high]);
    if (arr[mid] > arr[high]) swap_int(&arr[mid], &arr[high]);
    return mid;
}

/**
 * Recursive/iterative hybrid Quick Sort on arr[low..high].
 *
 * Uses median-of-three pivot selection, falls back to insertion sort
 * for sub-arrays smaller than 16, and recurses into the smaller
 * partition first (tail-call optimised via loop) to bound stack depth
 * to O(log N).
 *
 * @param arr   Array to sort in-place.
 * @param low   Start index (inclusive).
 * @param high  End index (inclusive).
 */
static void quick_sort_internal(int *arr, int low, int high) {
    while (low < high) {
        if (high - low < 16) {
            for (int i = low + 1; i <= high; i++) {
                int key = arr[i];
                int j = i - 1;
                while (j >= low && arr[j] > key) {
                    arr[j + 1] = arr[j];
                    j--;
                }
                arr[j + 1] = key;
            }
            return;
        }

        int p_idx = median_of_three(arr, low, high);
        int pivot = arr[p_idx];
        swap_int(&arr[p_idx], &arr[high]);

        int i = low;
        int j = high - 1;

        while (1) {
            while (i <= j && arr[i] < pivot) i++;
            while (j >= i && arr[j] > pivot) j--;
            if (i >= j) break;
            swap_int(&arr[i], &arr[j]);
            i++;
            j--;
        }

        swap_int(&arr[i], &arr[high]);

        if (i - low < high - i) {
            quick_sort_internal(arr, low, i - 1);
            low = i + 1;
        } else {
            quick_sort_internal(arr, i + 1, high);
            high = i - 1;
        }
    }
}

/**
 * Quick Sort — O(N log N) average, O(N^2) theoretical worst.
 *
 * In-place, not stable. Median-of-three pivot selection and insertion
 * sort cutoff make worst-case extremely unlikely on random data.
 *
 * @param arr  Array to sort in-place, ascending. No-op if NULL.
 * @param n    Number of elements. No-op if n <= 1.
 */
static void algo_quick_sort(int *arr, int n) {
    if (!arr || n <= 1) return;
    quick_sort_internal(arr, 0, n - 1);
}

/**
 * Recursive Merge Sort on arr[left..right] using a pre-allocated
 * auxiliary buffer.  Falls back to insertion sort for sub-arrays
 * smaller than 16 and skips merging when halves are already ordered.
 *
 * @param arr    Array to sort in-place.
 * @param aux    Pre-allocated auxiliary buffer of at least (right+1) ints.
 * @param left   Start index (inclusive).
 * @param right  End index (inclusive).
 */
static void merge_internal(int *arr, int *aux, int left, int right) {
    if (left >= right) return;

    if (right - left < 16) {
        for (int i = left + 1; i <= right; i++) {
            int key = arr[i];
            int j = i - 1;
            while (j >= left && arr[j] > key) {
                arr[j + 1] = arr[j];
                j--;
            }
            arr[j + 1] = key;
        }
        return;
    }

    int mid = left + (right - left) / 2;
    merge_internal(arr, aux, left, mid);
    merge_internal(arr, aux, mid + 1, right);

    if (arr[mid] <= arr[mid + 1]) return;

    int i = left;
    int j = mid + 1;
    int k = left;

    while (i <= mid && j <= right) {
        if (arr[i] <= arr[j]) {
            aux[k++] = arr[i++];
        } else {
            aux[k++] = arr[j++];
        }
    }
    while (i <= mid) aux[k++] = arr[i++];
    while (j <= right) aux[k++] = arr[j++];

    memcpy(&arr[left], &aux[left], (size_t)(right - left + 1) * sizeof(int));
}

/**
 * Merge Sort — O(N log N) guaranteed.
 *
 * Stable, requires O(N) auxiliary memory (heap-allocated).
 * Silently returns without sorting if allocation fails.
 *
 * @param arr  Array to sort in-place, ascending. No-op if NULL.
 * @param n    Number of elements. No-op if n <= 1.
 */
static void algo_merge_sort(int *arr, int n) {
    if (!arr || n <= 1) return;
    int *aux = (int *)malloc((size_t)n * sizeof(int));
    if (!aux) return;
    merge_internal(arr, aux, 0, n - 1);
    free(aux);
}

/**
 * Sifts element at index @p i down through a max-heap of size @p n
 * to restore the heap property.
 *
 * @param arr  Array representing the heap.
 * @param n    Heap size (only indices < n are considered).
 * @param i    Index of the element to sift down.
 */
static void heapify(int *arr, int n, int i) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    if (left < n && arr[left] > arr[largest])
        largest = left;

    if (right < n && arr[right] > arr[largest])
        largest = right;

    if (largest != i) {
        swap_int(&arr[i], &arr[largest]);
        heapify(arr, n, largest);
    }
}

/**
 * Heap Sort — O(N log N) guaranteed.
 *
 * In-place, not stable. Builds a max-heap then repeatedly extracts
 * the maximum.
 *
 * @param arr  Array to sort in-place, ascending. No-op if NULL.
 * @param n    Number of elements. No-op if n <= 1.
 */
static void algo_heap_sort(int *arr, int n) {
    if (!arr || n <= 1) return;

    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i);
    }

    for (int i = n - 1; i > 0; i--) {
        swap_int(&arr[0], &arr[i]);
        heapify(arr, i, 0);
    }
}

/**
 * Shell Sort — O(N^(3/2)) with Knuth's 3x+1 gap sequence.
 *
 * In-place, not stable. Significantly faster than pure insertion sort
 * on medium-sized arrays; benchmarked as sub-quadratic in practice.
 *
 * @param arr  Array to sort in-place, ascending. No-op if NULL.
 * @param n    Number of elements. No-op if n <= 1.
 */
static void algo_shell_sort(int *arr, int n) {
    if (!arr || n <= 1) return;
    int gap = 1;
    while (gap < n / 3) {
        gap = 3 * gap + 1;
    }
    while (gap > 0) {
        for (int i = gap; i < n; i++) {
            int key = arr[i];
            int j = i;
            while (j >= gap && arr[j - gap] > key) {
                arr[j] = arr[j - gap];
                j -= gap;
            }
            arr[j] = key;
        }
        gap /= 3;
    }
}

/**
 * Selection Sort — O(N^2) always.
 *
 * In-place, not stable. Performs the minimum number of swaps (N-1),
 * but always does O(N^2) comparisons regardless of input order.
 *
 * @param arr  Array to sort in-place, ascending. No-op if NULL.
 * @param n    Number of elements. No-op if n <= 1.
 */
static void algo_selection_sort(int *arr, int n) {
    if (!arr || n <= 1) return;
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++) {
            if (arr[j] < arr[min_idx]) {
                min_idx = j;
            }
        }
        if (min_idx != i) {
            swap_int(&arr[i], &arr[min_idx]);
        }
    }
}

/**
 * Bubble Sort — O(N^2) average/worst, O(N) best (already sorted).
 *
 * Stable, in-place. Includes an early-exit optimisation: terminates
 * immediately if a full pass completes with no swaps.
 *
 * @param arr  Array to sort in-place, ascending. No-op if NULL.
 * @param n    Number of elements. No-op if n <= 1.
 */
static void algo_bubble_sort(int *arr, int n) {
    if (!arr || n <= 1) return;
    for (int i = 0; i < n - 1; i++) {
        bool swapped = false;
        int limit = n - 1 - i;
        for (int j = 0; j < limit; j++) {
            if (arr[j] > arr[j + 1]) {
                swap_int(&arr[j], &arr[j + 1]);
                swapped = true;
            }
        }
        if (!swapped) break;
    }
}

/* ========================================================================= */
/* Registered Algorithms Metadata                                            */
/* ========================================================================= */

static const AlgorithmDef REGISTRY[] = {
    {
        .id = "quick",
        .name = "Quick Sort",
        .func = algo_quick_sort,
        .is_quadratic = false
    },
    {
        .id = "merge",
        .name = "Merge Sort",
        .func = algo_merge_sort,
        .is_quadratic = false
    },
    {
        .id = "heap",
        .name = "Heap Sort",
        .func = algo_heap_sort,
        .is_quadratic = false
    },
    {
        .id = "shell",
        .name = "Shell Sort",
        .func = algo_shell_sort,
        .is_quadratic = false
    },
    {
        .id = "insertion",
        .name = "Insertion Sort",
        .func = algo_insertion_sort,
        .is_quadratic = true
    },
    {
        .id = "selection",
        .name = "Selection Sort",
        .func = algo_selection_sort,
        .is_quadratic = true
    },
    {
        .id = "bubble",
        .name = "Bubble Sort",
        .func = algo_bubble_sort,
        .is_quadratic = true
    }
};

static const int REGISTRY_COUNT = sizeof(REGISTRY) / sizeof(REGISTRY[0]);

int get_algorithm_count(void) {
    return REGISTRY_COUNT;
}

const AlgorithmDef *get_algorithm(int index) {
    if (index >= 0 && index < REGISTRY_COUNT) {
        return &REGISTRY[index];
    }
    return NULL;
}
