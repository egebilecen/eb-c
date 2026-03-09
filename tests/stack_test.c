#include "test.h"

#define STACK_IMPLEMENTATION
#include "stack.h"

void stack_test()
{
    PRINT_TEST_FUNC();

    /* -------------------------------- Constants ------------------------------- */
    const size_t initial_capacity = 4;
    const size_t element_count = 10;
    int dummy = 69;

    /* ------------------------------- Initialize ------------------------------- */
    stack_t stack = STACK_INIT(int, initial_capacity);
    assert(stack.data != NULL);
    assert(stack.len == 0);

    /* ---------------------------------- Push ---------------------------------- */
    for (size_t i = 0; i < element_count; i++)
    {
        assert(stack_push(&stack, &i));
        assert(stack.len == i + 1);
    }

    assert(stack.len == element_count);

    /* ---------------------------------- Peek ---------------------------------- */
    int *top = (int *)stack_peek(&stack);
    assert(top != NULL);
    assert(*top == (int)(element_count - 1));

    /* ----------------------------------- Pop ---------------------------------- */
    for (size_t i = 0; i < element_count; i++)
    {
        int value;
        assert(stack_pop(&stack, &value));
        assert(value == (int)(element_count - 1 - i));
    }

    assert(stack.len == 0);
    assert(stack_pop(&stack, NULL) == false);

    /* ---------------------------------- Clear --------------------------------- */
    assert(stack_push(&stack, &dummy));
    assert(*(int *)stack_peek(&stack) == dummy);

    stack_clear(&stack);
    assert(stack_peek(&stack) == NULL);
    assert(stack.len == 0);

    /* ---------------------------------- Free ---------------------------------- */
    stack_free(&stack);
    assert(stack.data == NULL);
    assert(stack.len == 0);
}
