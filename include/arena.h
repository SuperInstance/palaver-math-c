#ifndef ARENA_H
#define ARENA_H

#include <stddef.h>

#define ARENA_BLOCK_SIZE (64 * 1024)

typedef struct ArenaBlock {
    struct ArenaBlock *next;
    size_t used;
    size_t capacity;
    char data[];
} ArenaBlock;

typedef struct {
    ArenaBlock *head;
    ArenaBlock *current;
} Arena;

void arena_init(Arena *a);
void *arena_alloc(Arena *a, size_t size);
void arena_free(Arena *a);

#endif /* ARENA_H */
