/* Override BENCHMARK_RUN_ALL before including benchmark.h (see benchmark.h for details) */
#define BENCHMARK_RUN_ALL 0

#include "benchmark.h"

int main(void) {
    return run_benchmark();
}
