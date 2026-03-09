#if !defined(VECTOR_H)
#define VECTOR_H

#ifndef TRACKED_FUNC_CALL
#define TRACKED_FUNC_CALL(func, args_call) func args_call
#endif // TRACKED_FUNC_CALL

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------- */
/*                                   Macros                                   */
/* -------------------------------------------------------------------------- */
#define VEC_INIT(type, capacity) vec_init(sizeof(type), capacity)
#define VEC_INIT_WITH_FLAGS(type, capacity, flags) vec_init_with_flags(sizeof(type), capacity, flags)

/* -------------------------------------------------------------------------- */
/*                                    Enums                                   */
/* -------------------------------------------------------------------------- */
typedef enum
{
    VEC_FLAG_NONE = 0,
    VEC_FLAG_AUTO_RESIZE = 1 << 0,         // Automatically resize the vector when capacity is reached.
    VEC_FLAG_CLEAR_MEMORY_ON_POP = 1 << 1, // Automatically clear memory of popped elements.
} vec_flags_e;

#define VEC_FLAG_DEFAULT (VEC_FLAG_AUTO_RESIZE | VEC_FLAG_CLEAR_MEMORY_ON_POP)

/* -------------------------------------------------------------------------- */
/*                                   Structs                                  */
/* -------------------------------------------------------------------------- */
typedef struct
{
    void *data;
    size_t len;
    size_t capacity;
    size_t element_size;
    vec_flags_e flags;
} vec_t;

/* -------------------------------------------------------------------------- */
/*                                  Functions                                 */
/* -------------------------------------------------------------------------- */
/// Initializes a vector with the initial capacity for the specified element size using default flags.
vec_t vec_init(size_t element_size, size_t initial_capacity);

/// Initializes a vector with the initial capacity for the specified element size and flags.
vec_t vec_init_with_flags(size_t element_size, size_t capacity, vec_flags_e flags);

/// Pushes a new element to the end of the vector.
/// Returns true on success, false if the vector is full and cannot be resized.
bool vec_push(vec_t *vec, const void *element);

/// Pops the last element from the vector. Optionally outputs the popped element.
/// Returns true on success, false if the vector is empty.
bool vec_pop(vec_t *vec, void *out_element);

/// Inserts an element at the specified index, shifting subsequent elements. 
/// Returns true on success, false if the index is out of bounds or the vector is full and cannot be resized.
bool vec_insert(vec_t *vec, size_t index, const void *element);

/// Removes the element at the specified index, shifting subsequent elements. Optionally outputs the removed element. 
/// Returns true on success, false if the index is out of bounds.
bool vec_remove(vec_t *vec, size_t index, void *out_element);

/// Retrieves a pointer to the element at the specified index without bounds checking. Behavior is undefined if out of bounds.
void *vec_get_unchecked(const vec_t *vec, size_t index);

/// Retrieves a pointer to the element at the specified index. Returns NULL if out of bounds.
void *vec_get(const vec_t *vec, size_t index);

/// Sets the element at the specified index to the provided value without bounds checking. Behavior is undefined if out of bounds.
void vec_set_unchecked(vec_t *vec, size_t index, const void *element);

/// Sets the element at the specified index to the provided value. Returns true on success.
bool vec_set(vec_t *vec, size_t index, const void *element);

/// Clears all elements from the vector. The capacity remains unchanged.
void vec_clear(vec_t *vec);

/// Frees the memory used by the vector and resets its fields.
void vec_free(vec_t *vec);

/* ----------------------------- Implementation ----------------------------- */
#ifdef VECTOR_IMPLEMENTATION
#undef VECTOR_IMPLEMENTATION

/* -------------------------------------------------------------------------- */
/*                              Private Functions                             */
/* -------------------------------------------------------------------------- */
static inline bool _vec_should_resize(const vec_t *vec)
{
    return vec->len == vec->capacity;
}

static bool _vec_resize(vec_t *vec)
{
    // Auto-resize is disabled.
    if (!(vec->flags & VEC_FLAG_AUTO_RESIZE))
        return false;

    size_t new_capacity = vec->capacity * 2;
    void *new_data = realloc(vec->data, vec->element_size * new_capacity);

    // Re-allocation failed.
    if (new_data == NULL)
        return false;

    vec->data = new_data;
    vec->capacity = new_capacity;
    return true;
}

/* -------------------------------------------------------------------------- */
/*                              Public Functions                              */
/* -------------------------------------------------------------------------- */
inline vec_t vec_init(size_t element_size, size_t initial_capacity)
{
    return vec_init_with_flags(element_size, initial_capacity, VEC_FLAG_DEFAULT);
}

vec_t vec_init_with_flags(size_t element_size, size_t capacity, vec_flags_e flags)
{
    return (vec_t){
        .data = TRACKED_FUNC_CALL(malloc, (element_size * capacity)),
        .len = 0,
        .capacity = capacity,
        .element_size = element_size,
        .flags = flags,
    };
}

inline bool vec_push(vec_t *vec, const void *element)
{
    return vec_insert(vec, vec->len, element);
}

bool vec_pop(vec_t *vec, void *out_element)
{
    if (vec->len == 0)
        return false;

    void *src = (char *)vec->data + ((vec->len - 1) * vec->element_size);

    // Copy the last element to the output and decrease the length.
    if (out_element != NULL)
        memcpy(out_element, src, vec->element_size);

    if (vec->flags & VEC_FLAG_CLEAR_MEMORY_ON_POP)
        memset(src, 0, vec->element_size);

    vec->len--;
    return true;
}

bool vec_insert(vec_t *vec, size_t index, const void *element)
{
    if (index > vec->len)
        return false;

    if (_vec_should_resize(vec) && !_vec_resize(vec))
        return false;

    void *src = (char *)vec->data + (index * vec->element_size);

    // Shift elements to the right to make space for the new element if we are not inserting at the end.
    if (index != vec->len)
    {
        void *dest = (char *)vec->data + ((index + 1) * vec->element_size);
        size_t num_bytes_to_move = (vec->len - index) * vec->element_size;
        memmove(dest, src, num_bytes_to_move);
    }

    // Copy the new element into the vector.
    memcpy(src, element, vec->element_size);
    vec->len++;

    return true;
}

bool vec_remove(vec_t *vec, size_t index, void *out_element)
{
    if (index >= vec->len)
        return false;

    // Removing the last element is equivalent to popping.
    if (index == vec->len - 1)
        return vec_pop(vec, out_element);

    // Optionally copy the element to be removed to the output.
    if (out_element != NULL)
    {
        void *src = (char *)vec->data + (index * vec->element_size);
        memcpy(out_element, src, vec->element_size);
    }

    // Shift elements to the left to fill the gap.
    void *dest = (char *)vec->data + (index * vec->element_size);
    void *src = (char *)vec->data + ((index + 1) * vec->element_size);
    size_t num_bytes_to_move = (vec->len - index - 1) * vec->element_size;

    memmove(dest, src, num_bytes_to_move);
    vec->len--;

    return true;
}

inline void *vec_get_unchecked(const vec_t *vec, size_t index)
{
    return (char *)vec->data + (index * vec->element_size);
}

void *vec_get(const vec_t *vec, size_t index)
{
    if (index >= vec->len)
        return NULL;

    return vec_get_unchecked(vec, index);
}

void vec_set_unchecked(vec_t *vec, size_t index, const void *element)
{
    void *dest = (char *)vec->data + (index * vec->element_size);
    memcpy(dest, element, vec->element_size);
}

bool vec_set(vec_t *vec, size_t index, const void *element)
{
    if (index >= vec->len)
        return false;

    vec_set_unchecked(vec, index, element);
    return true;
}

void vec_clear(vec_t *vec)
{
    if (vec == NULL || vec->data == NULL)
        return;

    memset(vec->data, 0, vec->element_size * vec->capacity);
    vec->len = 0;
}

void vec_free(vec_t *vec)
{
    TRACKED_FUNC_CALL(free, (vec->data));
    vec->data = NULL;
    vec->len = 0;
    vec->capacity = 0;
    vec->element_size = 0;
}

#endif // VECTOR_IMPLEMENTATION
#endif // VECTOR_H
