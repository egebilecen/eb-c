//! If `ARENA_ENABLE_MANAGER` is defined, the arena will keep track of allocated
//! entries and allow clearing them individually. This adds some overhead but
//! can be useful for certain use cases. This feature depends on `linked_list.h`
//! so be sure to add it to the include path if you enable it.
#if !defined(ARENA_H)
#define ARENA_H

#ifndef TRACKED_FUNC_CALL
#define TRACKED_FUNC_CALL(func, args_call) func args_call
#endif // TRACKED_FUNC_CALL

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef ARENA_ENABLE_MANAGER
#ifndef LINKED_LIST_IMPLEMENTATION
#define LINKED_LIST_IMPLEMENTATION
#endif
#include "linked_list.h"
#endif // ARENA_ENABLE_MANAGER

/* -------------------------------------------------------------------------- */
/*                                   Structs                                  */
/* -------------------------------------------------------------------------- */
#ifdef ARENA_ENABLE_MANAGER
typedef struct
{
    void *ptr;
    size_t size;
} arena_alloc_t;
#endif // ARENA_ENABLE_MANAGER

typedef struct arena_s
{
    void *memory;
    size_t size;
    size_t used;

#ifdef ARENA_ENABLE_MANAGER
    ll_list_t *allocations; // List of `arena_alloc_t`
    struct arena_s *next;   // Pointer to the next arena if this one is extended
#endif                      // ARENA_ENABLE_MANAGER
} arena_t;

/* -------------------------------------------------------------------------- */
/*                                  Functions                                 */
/* -------------------------------------------------------------------------- */
/// Initializes an arena with the specified size in bytes.
arena_t arena_init(size_t size);

/// Initializes a new arena on the heap and returns a pointer to it.
/// Returns NULL if the allocation fails.
arena_t *arena_new(size_t size);

/// Allocates a new chunk of memory in the arena and returns a pointer to it.
/// Returns NULL if the arena is full or not initialized.
/// WARNING: If `ARENA_ENABLE_MANAGER` is defined, NULL might also be returned
/// if the arena is fragmented and the allocation manager cannot find a suitable
/// location for the new entry.
void *arena_alloc(arena_t *arena, size_t size);

#ifdef ARENA_ENABLE_MANAGER
/// Internal function to create a new arena entry.
/// Not meant to be used directly.
arena_alloc_t *_arena_new_entry(void *ptr, size_t size);

/// Extends the arena by allocating a new arena and linking it to the existing
/// arena. This can be useful if you want to increase the capacity of the arena
/// without losing existing allocations.
bool arena_extend(arena_t *arena, size_t size);

/// Deallocates a chunk of memory in the arena. The `ptr` parameter must be a
/// pointer returned by a previous call to `arena_alloc` on
/// the same arena. Returns true if the deallocation was successful, false
/// otherwise (e.g., if the pointer is invalid or not found in the arena).
bool arena_dealloc(arena_t *arena, void *ptr);
#endif // ARENA_ENABLE_MANAGER

/// Frees the memory used by the stack allocated arena and resets its fields.
void arena_drop(arena_t *arena);

/// Frees the memory used by the heap allocated arena and resets its fields.
void arena_free(arena_t *arena);

/* ----------------------------- Implementation ----------------------------- */
#ifdef ARENA_IMPLEMENTATION
#undef ARENA_IMPLEMENTATION

/* -------------------------------------------------------------------------- */
/*                              Private Functions                             */
/* -------------------------------------------------------------------------- */
arena_alloc_t *_arena_new_entry(void *ptr, size_t size)
{
    void *new_entry = TRACKED_FUNC_CALL(malloc, (sizeof(arena_alloc_t)));
    if (new_entry == NULL)
        return NULL;

    arena_alloc_t *entry = (arena_alloc_t *)new_entry;
    entry->ptr = ptr;
    entry->size = size;

    return entry;
}

/* -------------------------------------------------------------------------- */
/*                              Public Functions                              */
/* -------------------------------------------------------------------------- */
arena_t arena_init(size_t size)
{
    return (arena_t){
        .memory = TRACKED_FUNC_CALL(malloc, (size)),
        .size = size,
        .used = 0,
#ifdef ARENA_ENABLE_MANAGER
        .allocations = ll_new(),
        .next = NULL
#endif // ARENA_ENABLE_MANAGER
    };
}

arena_t *arena_new(size_t size)
{
    arena_t *new_arena = TRACKED_FUNC_CALL(malloc, (sizeof(arena_t)));
    if (new_arena == NULL)
        return NULL;

    *new_arena = arena_init(size);
    return new_arena;
}

void *arena_alloc(arena_t *arena, size_t size)
{
    // Check if the arena has enough space for the new allocation. If not,
    /// return NULL. If the arena is extended, the allocation will be attempted
    /// in the next arena.
    if (arena->used + size > arena->size)
    {
#ifdef ARENA_ENABLE_MANAGER
        // Try to extend the arena if it's full.
        if (arena->next != NULL)
            return arena_alloc(arena->next, size);
#endif // ARENA_ENABLE_MANAGER

        return NULL;
    }

#ifdef ARENA_ENABLE_MANAGER
    // Look for a suitable memory location.
    void *mem = NULL;

    if (arena->allocations->size > 0)
    {
        // Check if we can allocate at the beginning of the arena
        // (before the first entry).
        void *end_addr = (char *)arena->memory + size;
        bool can_alloc = true;

        LL_FOR_EACH_DATA(arena->allocations->begin, data, arena_alloc_t *)
        {
            if (data->ptr < end_addr)
            {
                can_alloc = false;
                break;
            }
        }

        if (can_alloc)
        {
            mem = arena->memory;
            goto alloc;
        }

        LL_FOR_EACH_NODE(arena->allocations->begin, node)
        {
            arena_alloc_t *current = (arena_alloc_t *)node->data;
            char *current_end_addr = (char *)current->ptr + current->size;

            // Last or only entry.
            if (node->next == arena->allocations->begin)
            {
                // Check if the new entry can fit after the entry and before
                // the end of the arena.
                if (current_end_addr + size <=
                    (char *)arena->memory + arena->size)
                {
                    mem = current_end_addr;
                    break;
                }

                // Check if arena is extended.
                // If so, try to allocate in the next arena.
                if (arena->next != NULL)
                    mem = arena_alloc(arena->next, size);

                break;
            }

            arena_alloc_t *next = (arena_alloc_t *)node->next->data;

            size_t available_space = (size_t)((char *)next->ptr -
                                              current_end_addr);
            if (available_space >= size)
            {
                mem = current_end_addr;
                break;
            }
        }
    }
    else
    {
        mem = (char *)arena->memory;
    }

alloc:
    if (mem != NULL)
    {
        arena_alloc_t *new_entry = _arena_new_entry(mem, size);
        if (new_entry == NULL)
            return NULL;

        ll_push(arena->allocations, new_entry);
        arena->used += size;
    }
#else
    void *mem = (char *)arena->memory + arena->used;
    arena->used += size;
#endif // ARENA_ENABLE_MANAGER

    return mem;
}

#ifdef ARENA_ENABLE_MANAGER
bool arena_extend(arena_t *arena, size_t size)
{
    arena_t *new_arena = arena_new(size);
    if (new_arena == NULL)
        return false;

    arena->next = new_arena;
    return true;
}

bool arena_dealloc(arena_t *arena, void *ptr)
{
    LL_FOR_EACH_NODE(arena->allocations->begin, node)
    {
        arena_alloc_t *entry = (arena_alloc_t *)node->data;

        if (entry->ptr == ptr)
        {
            ll_remove(arena->allocations, node);
            arena->used -= entry->size;

            ll_free_node(node, true);
            return true;
        }
    }

    if (arena->next != NULL)
        return arena_dealloc(arena->next, ptr);

    return false;
}
#endif // ARENA_ENABLE_MANAGER

void arena_drop(arena_t *arena)
{
    // Reset arena fields.
    arena->size = 0;
    arena->used = 0;

    // Free the arena memory.
    TRACKED_FUNC_CALL(free, (arena->memory));
    arena->memory = NULL;

#ifdef ARENA_ENABLE_MANAGER
    // Free all allocation entries.
    ll_node_t *node = NULL;
    while ((node = ll_pop(arena->allocations)) != NULL)
        ll_free_node(node, true);

    // Free the allocations list itself.
    TRACKED_FUNC_CALL(free, (arena->allocations));
    arena->allocations = NULL;

    // Free the next arena if it exists.
    if (arena->next != NULL)
    {
        arena_free(arena->next);
        arena->next = NULL;
    }
#endif // ARENA_ENABLE_MANAGER
}

void arena_free(arena_t *arena)
{
    arena_drop(arena);
    TRACKED_FUNC_CALL(free, (arena));
}

#endif // ARENA_IMPLEMENTATION
#endif // ARENA_H
