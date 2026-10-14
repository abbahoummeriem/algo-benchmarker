#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#include "benchmark.h"
#include "algorithms.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#include <direct.h>
#else
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

/* ========================================================================= */
/* Cross-Platform High-Resolution Timing                                     */
/* ========================================================================= */

/**
 * Returns the current wall-clock time in seconds with nanosecond precision.
 *
 * Uses CLOCK_MONOTONIC_RAW on Linux (immune to NTP adjustments),
 * CLOCK_MONOTONIC as fallback, and QueryPerformanceCounter on Windows.
 *
 * @return  Current time in fractional seconds.
 */
static double timer_now_seconds(void) {
#ifdef _WIN32
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)freq.QuadPart;
#else
    struct timespec ts;
#if defined(CLOCK_MONOTONIC_RAW)
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
#else
    clock_gettime(CLOCK_MONOTONIC, &ts);
#endif
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
#endif
}

/* ========================================================================= */
/* Cross-Platform Image Viewer Launch                                        */
/* ========================================================================= */

/**
 * Opens an image file in the platform's default viewer.
 *
 * On Windows: tries ShellExecuteA, falls back to `start`.
 * On macOS:   uses `open`.
 * On Linux:   tries `xdg-open`, falls back to `gio open`.
 *
 * All commands are launched asynchronously (backgrounded) so they
 * don't block the benchmark process.
 *
 * @param image_path  Absolute or relative path to the image file.
 *                    No-op if NULL.
 */
static void open_in_default_viewer(const char *image_path) {
    if (!image_path) return;

#ifdef _WIN32
    HINSTANCE res = ShellExecuteA(NULL, "open", image_path, NULL, NULL, SW_SHOWNORMAL);
    if ((INT_PTR)res <= 32) {
        char cmd[1024];
        snprintf(cmd, sizeof(cmd), "start \"\" \"%s\"", image_path);
        system(cmd);
    }
#elif defined(__APPLE__)
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "open \"%s\" >/dev/null 2>&1 &", image_path);
    system(cmd);
#else
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "xdg-open \"%s\" >/dev/null 2>&1 &", image_path);
    if (system(cmd) != 0) {
        snprintf(cmd, sizeof(cmd), "gio open \"%s\" >/dev/null 2>&1 &", image_path);
        system(cmd);
    }
#endif
}

/* ========================================================================= */
/* Directory Helper                                                          */
/* ========================================================================= */

/**
 * Creates a directory if it does not already exist.
 *
 * Uses _mkdir on Windows and mkdir(0755) on POSIX.
 * Silently succeeds if the directory already exists.
 *
 * @param dir  Directory path to create. No-op if NULL or empty.
 */
static void ensure_dir(const char *dir) {
    if (!dir || strlen(dir) == 0) return;
#ifdef _WIN32
    _mkdir(dir);
#else
    struct stat st;
    if (stat(dir, &st) == -1) {
        mkdir(dir, 0755);
    }
#endif
}

/* ========================================================================= */
/* Gnuplot Script Generator                                                  */
/* ========================================================================= */

#define MAX_ALGOS 16
#define QUADRATIC_SIZE_CUTOFF 100000

static const char *PLOT_COLORS[] = {
    "#1f77b4", /* Quick: blue */
    "#ff7f0e", /* Merge: orange */
    "#2ca02c", /* Heap: green */
    "#d62728", /* Shell: crimson */
    "#9467bd", /* Insertion: purple */
    "#8c564b", /* Selection: brown */
    "#e377c2"  /* Bubble: pink */
};
static const int NUM_PLOT_COLORS = sizeof(PLOT_COLORS) / sizeof(PLOT_COLORS[0]);

static const int PLOT_POINT_TYPES[] = {
    7,  /* Quick: filled circle */
    5,  /* Merge: filled square */
    9,  /* Heap: filled triangle up */
    13, /* Shell: filled diamond */
    11, /* Insertion: filled triangle down */
    4,  /* Selection: open square */
    6   /* Bubble: open circle */
};

/**
 * Writes a self-contained Gnuplot script with embedded benchmark data.
 *
 * The generated script produces a single 2560x1440 PNG chart plotting
 * execution time vs. array size for all benchmarked algorithms, with
 * distinct colors and point types per algorithm.
 *
 * @param script_file  Output path for the .gp script.
 * @param png_file     Output path for the PNG chart (written into the script).
 * @param algos        Array of pointers to benchmarked algorithm descriptors.
 * @param algo_count   Number of algorithms in @p algos.
 * @param test_sizes   Array of tested array sizes (N values).
 * @param num_sizes    Number of entries in @p test_sizes.
 * @param timings      2D array [size_index][algo_index] of measured times.
 *                     Negative values indicate skipped/failed entries (output as NaN).
 * @param run_all      If true, X-axis spans to 1M; otherwise capped at 105K.
 */
static void write_gnuplot_script(const char *script_file,
                                 const char *png_file,
                                 const AlgorithmDef **algos,
                                 int algo_count,
                                 const int *test_sizes,
                                 int num_sizes,
                                 double timings[][MAX_ALGOS],
                                 bool run_all) {
    FILE *fp = fopen(script_file, "w");
    if (!fp) return;

    fprintf(fp, "# Gnuplot script auto-generated by algo_benchmarker\n");
    fprintf(fp, "set encoding utf8\n");

    /* Embed timing data block inline (no temporary .dat files) */
    fprintf(fp, "$BENCH_DATA << EOD\n");
    fprintf(fp, "# Size_N");
    for (int i = 0; i < algo_count; i++) {
        fprintf(fp, "\t%s", algos[i]->id);
    }
    fprintf(fp, "\n");

    for (int s = 0; s < num_sizes; s++) {
        fprintf(fp, "%d", test_sizes[s]);
        for (int a = 0; a < algo_count; a++) {
            if (timings[s][a] >= 0.0) {
                // fprintf(fp, "\t%.6f", timings[s][a]);
                                fprintf(fp, "\t%.9f", timings[s][a]);

            } else {
                fprintf(fp, "\tNaN");
            }
        }
        fprintf(fp, "\n");
    }
    fprintf(fp, "EOD\n\n");

    /* Single High-Resolution Linear Chart for All 7 Algorithms */
    fprintf(fp, "set term pngcairo size 2560,1440 enhanced font 'sans,20'\n");
    fprintf(fp, "set output '%s'\n", png_file);
    fprintf(fp, "set grid xtics ytics mxtics mytics lc rgb '#e0e0e0' lt 1 lw 1.2\n");
    fprintf(fp, "set tics font ',17'\n");
    fprintf(fp, "unset logscale\n");
    fprintf(fp, "set yrange [0:*]\n");
    fprintf(fp, "set format x \"%%h\"\n");
    fprintf(fp, "set xlabel 'Array Size (N)' font ',22' offset 0,-1.0\n");
    fprintf(fp, "set ylabel 'Execution Time (Seconds)' font ',22' offset -1.0,0\n");
    fprintf(fp, "set key left top box opaque font ',18' spacing 1.4\n\n");

    if (run_all) {
        fprintf(fp, "set xrange [0:1050000]\n");
        fprintf(fp, "set format y \"%%.0fs\"\n");
    } else {
        fprintf(fp, "set xrange [0:105000]\n");
        fprintf(fp, "set format y \"%%.2fs\"\n");
    }
    fprintf(fp, "set title 'Sorting Algorithm Benchmark: All 7 Algorithms (Linear Scale)' font ',26 bold'\n\n");

    fprintf(fp, "plot ");
    for (int i = 0; i < algo_count; i++) {
        const char *color = PLOT_COLORS[i % NUM_PLOT_COLORS];
        int pt = PLOT_POINT_TYPES[i % NUM_PLOT_COLORS];
        fprintf(fp, "$BENCH_DATA using 1:%d title '%s' with linespoints lw 3.5 pt %d ps 2.2 lc rgb '%s'%s",
                i + 2, algos[i]->name, pt, color,
                (i == algo_count - 1) ? "\n\n" : ", \\\n     ");
    }



    /* Added for the fastest algorithms plot for better readability */
    fprintf(fp, "set output 'plots/benchmark_fast.png'\n");
    fprintf(fp, "set xrange [0:1050000]\n");
    fprintf(fp, "set yrange [0:*]\n");
    fprintf(fp, "set format y \"%%.6fs\"\n");
fprintf(fp, "set title 'Sorting Algorithm Benchmark: Quick, Merge, Heap, and Shell (Up to 1 Million Elements)' font ',24 bold'\n\n");

    fprintf(fp, "plot ");
    bool first = true;
    for (int i = 0; i < algo_count; i++) {
        if (algos[i]->is_quadratic) continue;   /* Skip quadratic sorting algorithms */
        const char *color = PLOT_COLORS[i % NUM_PLOT_COLORS];
        int pt = PLOT_POINT_TYPES[i % NUM_PLOT_COLORS];
        if (!first) fprintf(fp, ", \\\n     ");
        first = false;
        fprintf(fp, "$BENCH_DATA using 1:%d title '%s' with linespoints lw 3.5 pt %d ps 2.2 lc rgb '%s'",
                i + 2, algos[i]->name, pt, color);
    }
    fprintf(fp, "\n");




    fclose(fp);
}

/* ========================================================================= */
/* Single Benchmark Entrypoint                                               */
/* ========================================================================= */

int run_benchmark_custom(bool config_run_all) {
    static const int TEST_SIZES[] = {
        1000, 5000, 10000,20000, 25000, 50000, 100000,150000, 250000, 500000, 1000000
    };
    static const int NUM_SIZES = sizeof(TEST_SIZES) / sizeof(TEST_SIZES[0]);

    int total_algos = get_algorithm_count();
    int algo_count = total_algos > MAX_ALGOS ? MAX_ALGOS : total_algos;
    const AlgorithmDef *algos[MAX_ALGOS];
    for (int i = 0; i < algo_count; i++) {
        algos[i] = get_algorithm(i);
    }

    const char *env_all = getenv("BENCH_ALL");
    bool env_flag = (env_all && (strcmp(env_all, "1") == 0 || strcmp(env_all, "true") == 0));
    bool run_all = config_run_all || env_flag;

    /* Print header banner */
    printf("\033[1;36m");
    printf("+-----------------------------------------------------------------------------------------------------------+\n");
    printf("|                 MINI ALGO BENCHMARKER & GNUPLOT RUNNER (1M SCALE)                                         |\n");
    printf("+-----------------------------------------------------------------------------------------------------------+\n");
    printf("\033[0m\n");

    if (run_all) {
        printf("\033[1;33m  MODE: EXHAUSTIVE FULL BENCHMARK (BENCHMARK_RUN_ALL = 1)\033[0m\n");
        printf("\033[0;90m     Running ALL 7 algorithms up to 1,000,000 elements without skipping.\033[0m\n\n");
    } else {
        printf("\033[1;32m  MODE: OPTIMIZED SKIP BENCHMARK (BENCHMARK_RUN_ALL = 0)\033[0m\n");
        printf("\033[0;90m     Skipping O(N^2) quadratic algorithms (Insertion, Bubble, Selection) for N > 100,000.\033[0m\n");
        printf("\033[0;90m     Scalable algorithms (Quick, Merge, Heap, Shell) run up to 1,000,000 elements.\033[0m\n\n");
    }

    printf("\033[1;36m> Running Benchmark (N = 1,000 to 1,000,000)\033[0m\n");
    printf("  %-10s", "Size (N)");
    for (int i = 0; i < algo_count; i++) {
        printf(" | %-11s", algos[i]->id);
    }
    printf("\n  ----------");
    for (int i = 0; i < algo_count; i++) {
        printf("+------------");
    }
    printf("\n");

    double timings[NUM_SIZES][MAX_ALGOS];
    for (int s = 0; s < NUM_SIZES; s++) {
        for (int a = 0; a < MAX_ALGOS; a++) {
            timings[s][a] = -1.0;
        }
    }

    /* Warm-up pass to prime CPU cache & governor */
    {
        int warmup_n = 500;
        int *w_arr = (int *)malloc(warmup_n * sizeof(int));
        if (w_arr) {
            generate_random_data(w_arr, warmup_n, 42);
            for (int a = 0; a < algo_count; a++) {
                int *tmp = (int *)malloc(warmup_n * sizeof(int));
                if (tmp) {
                    memcpy(tmp, w_arr, warmup_n * sizeof(int));
                    algos[a]->func(tmp, warmup_n);
                    free(tmp);
                }
            }
            free(w_arr);
        }
    }

    /* Benchmark Loop across all milestone sizes */
    for (int s = 0; s < NUM_SIZES; s++) {
        int n = TEST_SIZES[s];
        printf("  %-10d", n);

        int *src_data = (int *)malloc((size_t)n * sizeof(int));
        if (!src_data) {
            fprintf(stderr, "Error: Memory allocation failed for N=%d\n", n);
            break;
        }

        generate_random_data(src_data, n, 12345 + (unsigned int)s * 97);
        long long expected_sum = 0;
        long long expected_xor = 0;
        compute_checksums(src_data, n, &expected_sum, &expected_xor);

        for (int a = 0; a < algo_count; a++) {
            const AlgorithmDef *def = algos[a];

            if (!run_all && def->is_quadratic && n > QUADRATIC_SIZE_CUTOFF) {
                printf(" |    SKIP*   ");
                timings[s][a] = -1.0;
                continue;
            }

            int *work_buf = (int *)malloc((size_t)n * sizeof(int));
            if (!work_buf) {
                printf(" | \033[1;31m%-9s\033[0m ", "OOM");
                timings[s][a] = -1.0;
                continue;
            }

            int batch_iters = 1;
            if (!def->is_quadratic) {
                if (n <= 1000) batch_iters = 15;
                else if (n <= 5000) batch_iters = 5;
                else if (n <= 10000) batch_iters = 2;
            } else {
                if (n <= 1000) batch_iters = 5;
            }

            double total_time = 0.0;
            bool ok = true;
            int actual_runs = (def->is_quadratic && n > 25000) ? 1 : 3;

            for (int r = 0; r < actual_runs; r++) {
                double batch_time = 0.0;
                for (int b = 0; b < batch_iters; b++) {
                    memcpy(work_buf, src_data, (size_t)n * sizeof(int));
                    double t_start = timer_now_seconds();
                    def->func(work_buf, n);
                    double t_end = timer_now_seconds();
                    batch_time += (t_end - t_start);
                }
                total_time += (batch_time / (double)batch_iters);

                long long sorted_sum = 0, sorted_xor = 0;
                compute_checksums(work_buf, n, &sorted_sum, &sorted_xor);
                if (!is_sorted(work_buf, n) || sorted_sum != expected_sum || sorted_xor != expected_xor) {
                    ok = false;
                    break;
                }
            }

            free(work_buf);

            if (!ok) {
                printf(" | \033[1;31m%-9s\033[0m ", "FAIL");
                timings[s][a] = -1.0;
            } else {
                double avg_time = total_time / (double)actual_runs;
                printf(" | %9.5fs ", avg_time);
                timings[s][a] = avg_time;
            }
            fflush(stdout);
        }
        printf("\n");
        free(src_data);
    }

    if (!run_all) {
        printf("\n\033[0;90m  * Note: O(N^2) quadratic algorithms (Insertion, Selection, Bubble) skipped for N > 100,000\n");
        printf("    to avoid prohibitive execution times (Bubble Sort @ 1M takes ~45 minutes).\n");
        printf("    To run all tests including 1M: set '#define BENCHMARK_RUN_ALL 1' (or true) in main.c,\n");
        printf("    or run with environment variable: BENCH_ALL=1 ./bin/algo_benchmarker\033[0m\n");
    } else {
        printf("\n\033[1;33m  * Note: Exhaustive run completed for all 7 algorithms up to 1,000,000 elements.\033[0m\n");
    }

    ensure_dir("plots");
    const char *script_file = "plots/plot.gp";
    const char *png_file = "plots/benchmark.png";

    write_gnuplot_script(script_file, png_file, algos, algo_count, TEST_SIZES, NUM_SIZES, timings, run_all);

    char cmd[1024];
#ifdef _WIN32
    // snprintf(cmd, sizeof(cmd), "gnuplot \"%s\" 2>NUL", script_file);
        snprintf(cmd, sizeof(cmd), "gnuplot \"%s\"", script_file);

#else
    snprintf(cmd, sizeof(cmd), "gnuplot \"%s\" 2>/dev/null", script_file);
#endif
    int res = system(cmd);

    if (res == 0) {
        printf("\n\033[1;32mChart Generated:\033[0m\n");
        printf("  - Chart: \033[1;34m%s\033[0m\n", png_file);
        open_in_default_viewer(png_file);

        //ADDED FOR THE FASTEST ALGORITHMS PLOT
        open_in_default_viewer("plots/benchmark_fast.png");


    } else {
        printf("\n\033[1;33mNote: Gnuplot was not found or encountered an issue.\033[0m\n");
        printf("  Gnuplot script saved to: %s\n", script_file);
    }

    return 0;
}
