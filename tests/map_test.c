#include "test.h"
#include "track.h"

#define MAP_IMPLEMENTATION
#include "map.h"

void map_test()
{
    PRINT_TEST_FUNC();

    /* -------------------------------- Variables ------------------------------- */
    const size_t num_entries = 4;
    const char *keys[] = {"key1", "key2", "key3", "key4"};
    const char *values[] = {"value1", "value2", "value3", "value4"};

    /* ------------------------------- Initialize ------------------------------- */
    TRACKED_FUNC_RESET_COUNTER(calloc);
    TRACKED_FUNC_RESET_COUNTER(malloc);
    TRACKED_FUNC_RESET_COUNTER(free);

    map_t map = map_init(num_entries, map_hasher_fnv1a);
    assert(map.capacity == num_entries);

    /* ----------------------------------- Set ---------------------------------- */
    for (size_t i = 0; i < num_entries; i++)
        assert(map_set(&map, keys[i], (void *)values[i]));
    
    assert(!map_set(&map, "should_fail", (void *)values[0]));
    assert(!map_set(&map, "should_fail2", (void *)values[0]));

    /* ----------------------------------- Get ---------------------------------- */
    for (size_t i = 0; i < num_entries; i++)
    {
        void *value = map_get(&map, keys[i]);
        assert(value != NULL);
        assert(strcmp((char *)value, values[i]) == 0);
    }

    /* --------------------------------- Remove --------------------------------- */
    assert(map_remove(&map, keys[1]));
    assert(!map_remove(&map, keys[1]));
    assert(map_get(&map, keys[1]) == NULL);

    /* ---------------------------------- Free ---------------------------------- */
    map_free(&map);
    assert(map.container == NULL);
    assert(map.capacity == 0);

    assert(TRACKED_FUNC_GET_COUNTER(calloc) + TRACKED_FUNC_GET_COUNTER(malloc) ==
           TRACKED_FUNC_GET_COUNTER(free));
}