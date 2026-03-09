#include "test.h"
#include "track.h"

#define VECTOR_IMPLEMENTATION
#include "vector.h"

void vector_test()
{
    PRINT_TEST_FUNC();

    /* -------------------------------- Constants ------------------------------- */
    const size_t initial_capacity = 4;
    const size_t element_count = 10;
    const int dummy = 69; // Used for out-of-bounds tests.

    /* ------------------------------- Initialize ------------------------------- */
    TRACKED_FUNC_RESET_COUNTER(malloc);
    TRACKED_FUNC_RESET_COUNTER(free);

    vec_t vec = VEC_INIT(int, initial_capacity);
    assert(vec.data != NULL);
    assert(vec.len == 0);
    assert(vec.capacity == initial_capacity);

    /* ---------------------------------- Push ---------------------------------- */
    for (size_t i = 0; i < element_count; i++)
    {
        assert(vec_push(&vec, &i));
        assert(vec.len == i + 1);
    }

    assert(vec.len == element_count);
    assert(vec.capacity == 16);

    /* ----------------------------------- Get ---------------------------------- */
    for (size_t i = 0; i < vec.len; i++)
    {
        int *value = (int *)vec_get(&vec, i);
        assert(value != NULL);
        assert(*value == (int)i);
    }

    /* ----------------------------------- Set ---------------------------------- */
    for (size_t i = 0; i < vec.len; i++)
    {
        int new_value = (int)(i * 10);
        assert(vec_set(&vec, i, &new_value));
    }

    /* ----------------------------------- Pop ---------------------------------- */
    for (size_t i = 0; i < element_count; i++)
    {
        int value;
        size_t index = element_count - 1 - i;

        assert(vec_pop(&vec, &value));
        assert(value == ((int)index) * 10);
        assert(vec.len == element_count - 1 - i);

        // Verify that the popped element's memory has been cleared.
        int *popped_value = (int *)((char *)vec.data + (index * vec.element_size));
        assert(*popped_value == 0);
    }

    assert(vec.len == 0);

    /* --------------------------------- Insert --------------------------------- */
    for (size_t i = 0; i < element_count; i++)
    {
        assert(vec_insert(&vec, 0, &i)); // Insert at the beginning.
        assert(vec.len == i + 1);
    }

    assert(vec.len == element_count);

    /* --------------------------------- Remove --------------------------------- */
    for (size_t i = 0; i < element_count; i++)
    {
        int value;
        assert(vec_remove(&vec, 0, &value)); // Remove from the beginning.
        assert(value == (int)(element_count - 1 - i));
        assert(vec.len == element_count - 1 - i);
    }

    assert(vec.len == 0);

    /* ---------------------------- Out-of-bounds Get --------------------------- */
    assert(vec_get(&vec, 999999) == NULL);

    /* ---------------------------- Out-of-bounds Set --------------------------- */
    assert(!vec_set(&vec, 999999, &dummy));

    /* -------------------------- Out-of-bounds Insert -------------------------- */
    assert(!vec_insert(&vec, 999999, &dummy));

    /* ---------------------------------- Clear --------------------------------- */
    for (size_t i = 0; i < 3; i++)
        assert(vec_push(&vec, &i));

    assert(vec_get(&vec, 2) != NULL);
    vec_clear(&vec);
    assert(vec.len == 0);
    assert(vec_get(&vec, 2) == NULL);

    // Verify that the memory has been cleared.
    for (size_t i = 0; i < vec.capacity * vec.element_size; i++)
        assert(*((char *)vec.data + i) == 0);

    /* ---------------------------------- Free ---------------------------------- */
    vec_free(&vec);
    assert(vec.data == NULL);
    assert(vec.len == 0);
    assert(vec.capacity == 0);
    assert(vec.element_size == 0);

    TRACKED_FUNC_COMPARE_TWO_COUNTERS(malloc, free);

    /* ---------------------------------- Flags --------------------------------- */
    vec_t test_vec = VEC_INIT_WITH_FLAGS(int, 1, VEC_FLAG_NONE);

    /**
     * No Auto-Resize: The vector should not resize when capacity is reached, and the push should fail.
     */
    assert(vec_push(&test_vec, &dummy));
    assert(vec_push(&test_vec, &dummy) == false);

    /**
     * No Clear on Pop: Popped elements should not be cleared to zero.
     */
    assert(vec_pop(&test_vec, NULL));
    int *popped_value = (int *)((char *)test_vec.data + (test_vec.len * test_vec.element_size));
    assert(*popped_value == dummy);
}