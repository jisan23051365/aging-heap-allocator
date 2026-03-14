#include <stdio.h>
#include <stdlib.h>

/* =========================================================
   AGING HEAP ALLOCATOR
   ========================================================= */

typedef struct {
    int id;
    int size;
    int age;
} MemoryBlock;

void allocateBlock(MemoryBlock *block, int id, int size) {
    block->id = id;
    block->size = size;
    block->age = 0;
}

void ageBlock(MemoryBlock *block) {
    block->age++;
}

int shouldDeallocate(MemoryBlock block, int threshold) {
    return block.age > threshold;
}

int main() {
    MemoryBlock block;
    int threshold = 5;

    allocateBlock(&block, 1, 256);

    printf("Block Allocated | ID: %d | Size: %d\n", block.id, block.size);

    for (int cycle = 1; cycle <= 10; cycle++) {
        ageBlock(&block);
        printf("Cycle %d | Block Age: %d\n", cycle, block.age);

        if (shouldDeallocate(block, threshold)) {
            printf("Block expired and deallocated.\n");
            break;
        }
    }

    return 0;
}