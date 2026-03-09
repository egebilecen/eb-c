//! This library depends on `vector.h`, so be sure to add it to the include path.
#if !defined(QUEUE_H)
#define QUEUE_H

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifndef VECTOR_IMPLEMENTATION
#define VECTOR_IMPLEMENTATION
#endif // VECTOR_IMPLEMENTATION
#include "vector.h"

/* -------------------------------------------------------------------------- */
/*                                   Macros                                   */
/* -------------------------------------------------------------------------- */
#define QUEUE_INIT(type, capacity) queue_init(sizeof(type), capacity)
#define QUEUE_INIT_WITH_FLAGS(type, capacity, flags) queue_init_with_flags(sizeof(type), capacity, flags)

/* -------------------------------------------------------------------------- */
/*                                   Struct                                   */
/* -------------------------------------------------------------------------- */
typedef struct
{
    vec_t container;
    size_t front_index;
    size_t rear_index;
    size_t len;
} queue_t;

/* -------------------------------------------------------------------------- */
/*                                  Functions                                 */
/* -------------------------------------------------------------------------- */
/// Initializes a queue with the initial capacity for the specified element size using default flags.
queue_t queue_init(size_t element_size, size_t capacity);

/// Initializes a queue with the initial capacity for the specified element size and flags.
queue_t queue_init_with_flags(size_t element_size, size_t capacity, vec_flags_e flags);

/// Adds a new element to the rear of the queue. 
/// Returns true on success, false if the queue is full and cannot be resized.
bool queue_enqueue(queue_t *queue, const void *element);

/// Removes an element from the front of the queue.
/// Returns true on success, false if the queue is empty.
bool queue_dequeue(queue_t *queue, void *out_element);

/// Clears all elements from the queue and resets its indices. The capacity remains unchanged.
void queue_clear(queue_t *queue);

/// Frees the memory used by the queue.
void queue_free(queue_t *queue);

/* ----------------------------- Implementation ----------------------------- */
#ifdef QUEUE_IMPLEMENTATION
#undef QUEUE_IMPLEMENTATION

/* -------------------------------------------------------------------------- */
/*                              Public Functions                              */
/* -------------------------------------------------------------------------- */
queue_t queue_init(size_t element_size, size_t capacity)
{
    return queue_init_with_flags(element_size, capacity, VEC_FLAG_DEFAULT);
}

queue_t queue_init_with_flags(size_t element_size, size_t capacity, vec_flags_e flags)
{
    queue_t queue = {
        .container = vec_init_with_flags(element_size, capacity, flags),
        .front_index = 0,
        .rear_index = 0,
        .len = 0,
    };

    return queue;
}

bool queue_enqueue(queue_t *queue, const void *element)
{
    // If queue is empty or full, just push the element.
    // In case of a full queue, the vector will automatically resize if the resize flag is set.
    // If not, the push will fail and return false.
    if (queue->container.len == 0 || queue->rear_index == queue->front_index)
    {
        size_t old_capacity = queue->container.capacity;

        // Hack to make the vector appear full so next push will force it resize if auto-resize is enabled.
        if (queue->container.len != 0)
            queue->container.len = queue->container.capacity;

        bool result = vec_push(&queue->container, element);
        if (!result)
            return false;

        // Restore rear index to the end of the vector to maintain the correct order of elements after resizing.
        if (queue->container.len != 0)
            queue->rear_index = old_capacity;

        goto success;
    }

    vec_set_unchecked(&queue->container, queue->rear_index, element);

success:
    queue->rear_index = (queue->rear_index + 1) % queue->container.capacity;
    queue->len++;

    return true;
}

bool queue_dequeue(queue_t *queue, void *out_element)
{
    if (queue->len == 0)
        return false;

    if (out_element != NULL)
    {
        void *src = vec_get_unchecked(&queue->container, queue->front_index);
        memcpy(out_element, src, queue->container.element_size);
    }

    queue->front_index = (queue->front_index + 1) % queue->container.capacity;
    queue->len--;

    return true;
}

void queue_clear(queue_t *queue)
{
    vec_clear(&queue->container);
    queue->front_index = 0;
    queue->rear_index = 0;
    queue->len = 0;
}

void queue_free(queue_t *queue)
{
    vec_free(&queue->container);
}

#endif // QUEUE_IMPLEMENTATION
#endif // QUEUE_H
