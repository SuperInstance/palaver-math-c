#include "arena.h"
#include <stdlib.h>
#include <string.h>

void arena_init(Arena *a) {
    a->head = NULL;
    a->current = NULL;
}

static ArenaBlock *arena_new_block(size_t min_size) {
    size_t capacity = min_size > ARENA_BLOCK_SIZE ? min_size : ARENA_BLOCK_SIZE;
    ArenaBlock *block = (ArenaBlock *)malloc(sizeof(ArenaBlock) + capacity);
    if (!block) return NULL;
    block->next = NULL;
    block->used = 0;
    block->capacity = capacity;
    return block;
}

void *arena_alloc(Arena *a, size_t size) {
    /* Align to 8 bytes */
    size = (size + 7) & ~(size_t)7;

    if (!a->current || a->current->used + size > a->current->capacity) {
        ArenaBlock *block = arena_new_block(size);
        if (!block) return NULL;
        if (a->current) {
            a->current->next = block;
        } else {
            a->head = block;
        }
        a->current = block;
    }

    void *ptr = a->current->data + a->current->used;
    a->current->used += size;
    memset(ptr, 0, size);
    return ptr;
}

void arena_free(Arena *a) {
    ArenaBlock *block = a->head;
    while (block) {
        ArenaBlock *next = block->next;
        free(block);
        block = next;
    }
    a->head = NULL;
    a->current = NULL;
}
