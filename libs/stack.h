//! This library depends on `vector.h`, so be sure to add it to the include path.
#if !defined(STACK_H)
#define STACK_H

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
#define STACK_INIT(type, capacity) stack_init(sizeof(type), capacity)
#define STACK_INIT_WITH_FLAGS(type, capacity, flags) stack_init_with_flags(sizeof(type), capacity, flags)

/* -------------------------------------------------------------------------- */
/*                                    Types                                   */
/* -------------------------------------------------------------------------- */
typedef vec_t stack_t;

/* -------------------------------------------------------------------------- */
/*                                  Functions                                 */
/* -------------------------------------------------------------------------- */
/// Initializes a stack with the initial capacity for the specified element size using default flags.
stack_t stack_init(size_t element_size, size_t capacity);

/// Initializes a stack with the initial capacity for the specified element size and flags.
stack_t stack_init_with_flags(size_t element_size, size_t capacity, vec_flags_e flags);

/// Pushes a new element to the top of the stack.
/// Returns true on success, false if the stack is full and cannot be resized.
bool stack_push(stack_t *stack, const void *element);

/// Pops the top element from the stack. Optionally outputs the popped element.
/// Returns true on success, false if the stack is empty.
bool stack_pop(stack_t *stack, void *out_element);

/// Returns a pointer to the top element of the stack without removing it. Returns NULL if the stack is empty.
void *stack_peek(const stack_t *stack);

/// Clears all elements from the stack and resets its length to zero. The capacity remains unchanged.
void stack_clear(stack_t *stack);

/// Frees the memory used by the stack and resets its fields.
void stack_free(stack_t *stack);

/* ----------------------------- Implementation ----------------------------- */
#ifdef STACK_IMPLEMENTATION
#undef STACK_IMPLEMENTATION

/* -------------------------------------------------------------------------- */
/*                              Public Functions                              */
/* -------------------------------------------------------------------------- */
inline stack_t stack_init(size_t element_size, size_t capacity)
{
    return stack_init_with_flags(element_size, capacity, VEC_FLAG_DEFAULT);
}

stack_t stack_init_with_flags(size_t element_size, size_t capacity, vec_flags_e flags)
{
    return vec_init_with_flags(element_size, capacity, flags);
}

bool stack_push(stack_t *stack, const void *element)
{
    return vec_push(stack, element);
}

bool stack_pop(stack_t *stack, void *out_element)
{
    return vec_pop(stack, out_element);
}

void *stack_peek(const stack_t *stack)
{
    if (stack == NULL || stack->data == NULL || stack->len == 0)
        return NULL;

    return vec_get(stack, stack->len - 1);
}

void stack_clear(stack_t *stack)
{
    vec_clear(stack);
}

void stack_free(stack_t *stack)
{
    vec_free(stack);
}

#endif // STACK_IMPLEMENTATION
#endif // STACK_H
