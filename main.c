/*
 * main.c — Demonstration of the Aging Heap Allocator
 *
 * This program walks through several scenarios:
 *   1. Basic allocation and manual free
 *   2. Automatic expiry via aging (tick-based)
 *   3. Per-block custom age limit
 *   4. Stats reporting at each stage
 */

#include "aging_heap.h"

#include <stdio.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Helpers
 * ---------------------------------------------------------------------- */

static void separator(const char *title)
{
    printf("\n--- %s ---\n", title);
}

/* -------------------------------------------------------------------------
 * Scenarios
 * ---------------------------------------------------------------------- */

/*
 * Scenario 1: Allocate several blocks, manually free one, then run ticks
 * until the rest expire naturally.
 */
static void scenario_basic(void)
{
    separator("Scenario 1: Basic allocation and natural expiry");

    AgingHeap heap;
    if (aging_heap_init(&heap, 3) != 0) {  /* default max age = 3 cycles */
        printf("ERROR: aging_heap_init failed\n");
        return;
    }

    /* Allocate three blocks of different sizes. */
    char   *msg   = (char *)  aging_heap_alloc(&heap, 64,  0);
    int    *nums  = (int *)   aging_heap_alloc(&heap, sizeof(int) * 8, 0);
    double *vals  = (double *)aging_heap_alloc(&heap, sizeof(double) * 4, 0);

    if (!msg || !nums || !vals) {
        printf("ERROR: allocation failed\n");
        aging_heap_destroy(&heap);
        return;
    }

    strncpy(msg, "Hello, aging heap!", 63);
    msg[63] = '\0';
    for (int i = 0; i < 8; i++) nums[i] = i * 10;
    for (int i = 0; i < 4; i++) vals[i] = i * 1.5;

    printf("Allocated 3 blocks.  msg=\"%s\"\n", msg);
    aging_heap_stats(&heap);

    /* Manually free the double array before it expires. */
    printf("\n[manual free] freeing double array\n");
    aging_heap_free(&heap, vals);
    aging_heap_stats(&heap);

    /* Tick three times — remaining blocks expire after cycle 3. */
    for (unsigned int t = 1; t <= 4; t++) {
        unsigned int expired = aging_heap_tick(&heap);
        printf("\n[tick %u] expired blocks this tick: %u\n", t, expired);
        aging_heap_stats(&heap);
    }

    aging_heap_destroy(&heap);
}

/*
 * Scenario 2: Per-block custom age limits.
 * One block lives for 2 cycles, another for 5.
 */
static void scenario_custom_age(void)
{
    separator("Scenario 2: Per-block custom age limits");

    AgingHeap heap;
    aging_heap_init(&heap, 10);  /* high default — we override per block */

    void *short_lived = aging_heap_alloc(&heap, 128, 2);  /* expires after 2 */
    void *long_lived  = aging_heap_alloc(&heap, 256, 5);  /* expires after 5 */

    if (!short_lived || !long_lived) {
        printf("ERROR: allocation failed\n");
        aging_heap_destroy(&heap);
        return;
    }

    printf("Allocated: short-lived (max_age=2) and long-lived (max_age=5)\n");

    for (unsigned int t = 1; t <= 6; t++) {
        unsigned int expired = aging_heap_tick(&heap);
        printf("[tick %u] auto-freed: %u block(s)\n", t, expired);
    }

    aging_heap_stats(&heap);
    aging_heap_destroy(&heap);
}

/*
 * Scenario 3: Double-free guard.
 * Freeing the same pointer twice should return -1 the second time.
 */
static void scenario_double_free(void)
{
    separator("Scenario 3: Double-free guard");

    AgingHeap heap;
    aging_heap_init(&heap, 5);

    void *p = aging_heap_alloc(&heap, 32, 0);
    if (!p) {
        printf("ERROR: allocation failed\n");
        aging_heap_destroy(&heap);
        return;
    }

    int r1 = aging_heap_free(&heap, p);
    int r2 = aging_heap_free(&heap, p);  /* should fail */

    printf("First  free: %s (expected 0)\n",  r1 == 0  ? "OK" : "FAIL");
    printf("Second free: %s (expected -1)\n", r2 == -1 ? "OK" : "FAIL");

    aging_heap_destroy(&heap);
}

/*
 * Scenario 4: High-volume allocation — allocate many blocks and watch them
 * age out over time.
 */
static void scenario_high_volume(void)
{
    separator("Scenario 4: High-volume allocation");

#define N_BLOCKS 20
#define MAX_AGE  4

    AgingHeap heap;
    aging_heap_init(&heap, MAX_AGE);

    for (int i = 0; i < N_BLOCKS; i++) {
        void *p = aging_heap_alloc(&heap, (size_t)(32 * (i + 1)), 0);
        if (!p) {
            printf("WARNING: allocation %d failed\n", i);
        }
    }

    printf("Allocated %d blocks (max_age=%d each)\n", N_BLOCKS, MAX_AGE);
    aging_heap_stats(&heap);

    unsigned int total_expired = 0;
    for (unsigned int t = 1; t <= (unsigned int)(MAX_AGE + 1); t++) {
        unsigned int expired = aging_heap_tick(&heap);
        total_expired += expired;
        printf("[tick %u] auto-freed: %u — cumulative: %u\n",
               t, expired, total_expired);
    }

    aging_heap_stats(&heap);
    aging_heap_destroy(&heap);

#undef N_BLOCKS
#undef MAX_AGE
}

/* -------------------------------------------------------------------------
 * Entry point
 * ---------------------------------------------------------------------- */

int main(void)
{
    printf("=========================================\n");
    printf("   Aging Heap Allocator — Demo Program   \n");
    printf("=========================================\n");

    scenario_basic();
    scenario_custom_age();
    scenario_double_free();
    scenario_high_volume();

    printf("\nDone.\n");
    return 0;
}
