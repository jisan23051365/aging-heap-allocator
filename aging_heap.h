/*
 * aging_heap.h — Aging Heap Allocator
 *
 * A simulated heap where every allocated block carries an age counter.
 * Each call to aging_heap_tick() advances the simulation by one cycle:
 *   - all live blocks are incremented by one age unit
 *   - any block whose age reaches its per-block (or global) threshold is
 *     automatically freed
 */

#ifndef AGING_HEAP_H
#define AGING_HEAP_H

#include <stddef.h>

/* -------------------------------------------------------------------------
 * Data structures
 * ---------------------------------------------------------------------- */

/*
 * A single memory block tracked by the allocator.
 * Blocks are kept in a singly-linked intrusive list owned by AgingHeap.
 */
typedef struct MemBlock {
    void            *data;        /* pointer to the allocated payload       */
    size_t           size;        /* payload size in bytes                  */
    unsigned int     age;         /* current age in simulation cycles       */
    unsigned int     max_age;     /* age limit — auto-freed when age>=max_age */
    int              is_free;     /* 1 if this slot has been freed          */
    struct MemBlock *next;        /* intrusive linked-list link             */
} MemBlock;

/*
 * The heap allocator instance.
 * All MemBlock records are linked through ->next starting at ->head.
 */
typedef struct {
    MemBlock    *head;            /* first block in the list (or NULL)      */
    size_t       total_allocated; /* running total of bytes ever allocated  */
    size_t       live_bytes;      /* bytes currently in live (non-free) blocks */
    unsigned int cycle;           /* current simulation cycle counter       */
    unsigned int default_max_age; /* default age limit used when 0 is passed
                                     to aging_heap_alloc()                  */
} AgingHeap;

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

/*
 * aging_heap_init — initialise *heap* with the given default age threshold.
 *
 * default_max_age: number of cycles a block lives before auto-deallocation.
 *                  Must be > 0.
 * Returns 0 on success, -1 if arguments are invalid.
 */
int aging_heap_init(AgingHeap *heap, unsigned int default_max_age);

/*
 * aging_heap_alloc — allocate *size* bytes and return a pointer to the payload.
 *
 * max_age: per-block age limit.  Pass 0 to use heap->default_max_age.
 * Returns a non-NULL pointer on success, NULL on failure.
 */
void *aging_heap_alloc(AgingHeap *heap, size_t size, unsigned int max_age);

/*
 * aging_heap_free — manually free the block whose payload is *ptr*.
 *
 * Marks the block as free immediately (age is preserved for diagnostic
 * purposes but the slot is considered released).
 * Returns 0 on success, -1 if *ptr* is not found or already free.
 */
int aging_heap_free(AgingHeap *heap, void *ptr);

/*
 * aging_heap_tick — advance the simulation by one cycle.
 *
 * Every live block is aged by one unit.  Any block whose age reaches or
 * exceeds its max_age is automatically freed.
 * Returns the number of blocks that were auto-freed in this tick.
 */
unsigned int aging_heap_tick(AgingHeap *heap);

/*
 * aging_heap_stats — print a human-readable summary to stdout.
 */
void aging_heap_stats(const AgingHeap *heap);

/*
 * aging_heap_destroy — release all resources owned by *heap*.
 *
 * After this call *heap* must not be used again without re-initialisation.
 */
void aging_heap_destroy(AgingHeap *heap);

#endif /* AGING_HEAP_H */
