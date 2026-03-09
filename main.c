#define ENABLE_FUNCTION_TRACKING
#include "tests/arena_test.c"
#include "tests/vector_test.c"
#include "tests/stack_test.c"
#include "tests/queue_test.c"
#include "tests/linked_list_test.c"
#include "tests/map_test.c"

int main()
{
    linked_list_test();
    vector_test();
    stack_test();
    arena_test();
    dynamic_queue_test();
    static_queue_test();
    map_test();

    printf("✓ All tests passed!\n");
    return 0;
}
