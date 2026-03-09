#include "test.h"

#define QUEUE_IMPLEMENTATION
#include "queue.h"

void dynamic_queue_test()
{
    PRINT_TEST_FUNC();

    /* -------------------------------- Constants ------------------------------- */
    const size_t initial_capacity = 4;
    const size_t num_elements = 10;
    const int dummy = 69;

    /* -------------------------------- Initilize ------------------------------- */
    queue_t queue = QUEUE_INIT(int, initial_capacity);
    assert(queue.container.data != NULL);
    assert(queue.front_index == 0);
    assert(queue.rear_index == 0);
    assert(queue.len == 0);

    /* --------------------------------- Enqueue -------------------------------- */
    for (int i = 0; i < 10; i++)
        assert(queue_enqueue(&queue, &i));

    assert(queue.rear_index == num_elements);
    assert(queue.len == num_elements);

    /* --------------------------------- Dequeue -------------------------------- */
    for (size_t i = 0; i < 10; i++)
    {
        int dequeued_element;
        assert(queue_dequeue(&queue, (void *)&dequeued_element));
        assert(dequeued_element == (int)i);
        assert(queue.front_index == (i + 1));
        assert(queue.len == (num_elements - 1 - i));
    }

    assert(queue_dequeue(&queue, NULL) == false);
    assert(queue.front_index == num_elements);
    assert(queue.len == 0);

    /* ---------------------------------- Clear --------------------------------- */
    queue_enqueue(&queue, &dummy);
    queue_clear(&queue);
    assert(queue.front_index == 0);
    assert(queue.rear_index == 0);
    assert(queue.len == 0);
    assert(queue_dequeue(&queue, NULL) == false);
}

void static_queue_test()
{
    PRINT_TEST_FUNC();

    /* -------------------------------- Constants ------------------------------- */
    const size_t capacity = 4;
    const size_t num_elements = 6;

    /* -------------------------------- Initilize ------------------------------- */
    queue_t queue = QUEUE_INIT_WITH_FLAGS(int, capacity, VEC_FLAG_NONE);
    assert(queue.container.data != NULL);

    /* --------------------------------- Enqueue -------------------------------- */
    for (size_t i = 0; i < capacity; i++)
        assert(queue_enqueue(&queue, &i));

    // Queue is full, next enqueues should fail.
    for (size_t i = capacity; i < num_elements; i++)
        assert(queue_enqueue(&queue, &i) == false);
}