/*
 * aging_heap.c — Aging Heap Allocator implementation
 */

#include "aging_heap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Internal helpers
 * ---------------------------------------------------------------------- */

/* Allocate and zero-initialise a new MemBlock wrapper. */
static MemBlock *block_new(size_t size, unsigned int max_age)
{
    MemBlock *blk = (MemBlock *)malloc(sizeof(MemBlock));
    if (!blk) {
        return NULL;
    }

    blk->data = malloc(size);
    if (!blk->data) {
        free(blk);
        return NULL;
    }

    memset(blk->data, 0, size);
    blk->size    = size;
    blk->age     = 0;
    blk->max_age = max_age;
    blk->is_free = 0;
    blk->next    = NULL;
    return blk;
}

/* Release the payload and the wrapper of a single block. */
static void block_delete(MemBlock *blk)
{
    if (!blk) {
        return;
    }
    free(blk->data);
    blk->data = NULL;
    free(blk);
}

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

int aging_heap_init(AgingHeap *heap, unsigned int default_max_age)
{
    if (!heap || default_max_age == 0) {
        return -1;
    }

    heap->head            = NULL;
    heap->total_allocated = 0;
    heap->live_bytes      = 0;
    heap->cycle           = 0;
    heap->default_max_age = default_max_age;
    return 0;
}

void *aging_heap_alloc(AgingHeap *heap, size_t size, unsigned int max_age)
{
    if (!heap || size == 0) {
        return NULL;
    }

    unsigned int effective_max_age = (max_age == 0) ? heap->default_max_age
                                                     : max_age;

    MemBlock *blk = block_new(size, effective_max_age);
    if (!blk) {
        return NULL;
    }

    /* Prepend to the list — O(1). */
    blk->next  = heap->head;
    heap->head = blk;

    heap->total_allocated += size;
    heap->live_bytes      += size;

    return blk->data;
}

int aging_heap_free(AgingHeap *heap, void *ptr)
{
    if (!heap || !ptr) {
        return -1;
    }

    for (MemBlock *cur = heap->head; cur != NULL; cur = cur->next) {
        if (cur->data == ptr) {
            if (cur->is_free) {
                /* Already freed — double-free guard. */
                return -1;
            }
            cur->is_free     = 1;
            heap->live_bytes -= cur->size;
            return 0;
        }
    }

    /* Pointer not found. */
    return -1;
}

unsigned int aging_heap_tick(AgingHeap *heap)
{
    if (!heap) {
        return 0;
    }

    heap->cycle++;
    unsigned int expired = 0;

    for (MemBlock *cur = heap->head; cur != NULL; cur = cur->next) {
        if (cur->is_free) {
            continue;
        }

        cur->age++;

        if (cur->age >= cur->max_age) {
            cur->is_free     = 1;
            heap->live_bytes -= cur->size;
            expired++;
        }
    }

    return expired;
}

void aging_heap_stats(const AgingHeap *heap)
{
    if (!heap) {
        printf("[aging_heap] stats: (null heap)\n");
        return;
    }

    unsigned int live  = 0;
    unsigned int freed = 0;

    for (const MemBlock *cur = heap->head; cur != NULL; cur = cur->next) {
        if (cur->is_free) {
            freed++;
        } else {
            live++;
        }
    }

    printf("=== Aging Heap Stats (cycle %u) ===\n", heap->cycle);
    printf("  Default max age : %u cycles\n",   heap->default_max_age);
    printf("  Total allocated : %zu bytes\n",   heap->total_allocated);
    printf("  Live bytes      : %zu bytes\n",   heap->live_bytes);
    printf("  Live blocks     : %u\n",          live);
    printf("  Freed blocks    : %u\n",          freed);
    printf("===================================\n");
}

void aging_heap_destroy(AgingHeap *heap)
{
    if (!heap) {
        return;
    }

    MemBlock *cur = heap->head;
    while (cur) {
        MemBlock *next = cur->next;
        block_delete(cur);
        cur = next;
    }

    heap->head            = NULL;
    heap->total_allocated = 0;
    heap->live_bytes      = 0;
    heap->cycle           = 0;
}
