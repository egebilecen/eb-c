#include "test.h"
#include "track.h"

#define ARENA_IMPLEMENTATION
#define ARENA_ENABLE_MANAGER
#include "arena.h"

void list_allocations(arena_t *arena)
{
    printf("\n--- Arena State ---\n");
    printf("Total size: %zu bytes, Used: %zu bytes, Free: %zu bytes\n", arena->size, arena->used, arena->size - arena->used);
    printf("Arena base address: 0x%p\n", arena->memory);

    printf("\n--- Allocations ---\n");
    LL_FOR_EACH_DATA(arena->allocations->begin, data, arena_alloc_t *)
    {
        printf("Allocation: ptr=0x%p (offset %zu), size=%zu\n", data->ptr, (size_t)((char *)data->ptr - (char *)arena->memory), data->size);
    }
}

void arena_test()
{
    PRINT_TEST_FUNC();

    /* -------------------------------- Constants ------------------------------- */
    const size_t arena_size = 128;

    /* ------------------------------- Initialize ------------------------------- */
    TRACKED_FUNC_RESET_COUNTER(malloc);
    TRACKED_FUNC_RESET_COUNTER(free);

    arena_t arena = arena_init(arena_size);
    assert(arena.memory != NULL);
    assert(arena.size == arena_size);
    assert(arena.used == 0);
    assert(arena.next == NULL);

    /* -------------------------------- Allocate -------------------------------- */
    void *alloc_16 = NULL;
    void *alloc_4_1 = NULL;
    void *alloc_4_2 = NULL;

    assert(arena_alloc(&arena, 64));
    assert(arena_alloc(&arena, 32));
    assert((alloc_16 = arena_alloc(&arena, 16)));
    assert(arena_alloc(&arena, 8));
    assert((alloc_4_1 = arena_alloc(&arena, 4)));
    assert((alloc_4_2 = arena_alloc(&arena, 4)));
    assert(arena_alloc(&arena, 16) == NULL);

    assert(arena.used == 128);

    /* ------------------------------- De-allocate ------------------------------ */
    assert(arena_dealloc(&arena, alloc_4_1));
    assert(arena_dealloc(&arena, alloc_4_2));
    assert(arena_dealloc(&arena, alloc_16));
    assert(arena.used == 104);

    size_t i = 0;
    LL_FOR_EACH_DATA(arena.allocations->begin, data, arena_alloc_t *)
    {
        switch (i++)
        {
        case 0:
            assert(data->size == 64);
            break;

        case 1:
            assert(data->size == 32);
            break;

        case 2:
            assert(data->size == 8);
            break;

        default:
            assert(false);
        }
    }

    /* ------------------------------ Fragmentation ----------------------------- */
    // Memory is fragmented. These should fail.
    assert(arena_alloc(&arena, 20) == NULL);
    assert(arena_alloc(&arena, 24) == NULL);

    // This should succeed, as it fits in the 32-byte hole(s).
    assert(arena_alloc(&arena, 16) != NULL);
    assert(arena_alloc(&arena, 2) != NULL);
    assert(arena_alloc(&arena, 6) != NULL);

    // No more space left.
    assert(arena_alloc(&arena, 1) == NULL);

    /* ------------------------------ Extend Arena ------------------------------ */
    assert(arena_extend(&arena, 16));
    assert(arena.next != NULL);

    /* ------------------------ Extended Arena - Allocate ----------------------- */
    void *extended_alloc_8 = NULL;
    void *extended_alloc_4 = NULL;

    assert((extended_alloc_8 = arena_alloc(&arena, 8)) != NULL);
    assert((extended_alloc_4 = arena_alloc(&arena, 4)) != NULL);
    assert(arena_alloc(&arena, 2) != NULL);
    assert(arena_alloc(&arena, 2) != NULL);
    assert(arena_alloc(&arena, 1) == NULL);

    /* ---------------------- Extended Arena - De-allocate ---------------------- */
    assert(arena_dealloc(&arena, extended_alloc_8));
    assert(arena_dealloc(&arena, extended_alloc_4));

    assert(arena_alloc(&arena, 12) != NULL);
    assert(arena_alloc(&arena, 1) == NULL);

    /* ---------------------------------- Free ---------------------------------- */
    arena_drop(&arena);
    assert(arena.memory == NULL);
    assert(arena.size == 0);
    assert(arena.used == 0);
    assert(arena.next == NULL);

    TRACKED_FUNC_COMPARE_TWO_COUNTERS(malloc, free);
}
