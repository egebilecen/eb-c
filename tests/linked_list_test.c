#include "test.h"
#include "track.h"

#define LINKED_LIST_IMPLEMENTATION
#include "linked_list.h"

#define DATA_ARRAY_SIZE 256

void copy_list_to_array(ll_node_t *head, size_t *array, size_t array_size)
{
    size_t i = 0;
    LL_FOR_EACH_DATA(head, data, size_t *)
    {
        if (i >= array_size)
        {
            printf("WARNING: Array size is smaller than the number of nodes in the list."
                   "Stopping copy to prevent out-of-bounds access.\n");
            break;
        }

        array[i] = *data;
        i++;
    }
}

void linked_list_test()
{
    PRINT_TEST_FUNC();

    /* -------------------------------- Variables ------------------------------- */
    const size_t num_nodes = 16;
    size_t dummy = 69;
    size_t dummy2 = 96;
    size_t i;

    /* ------------------------------- Create List ------------------------------ */
    TRACKED_FUNC_RESET_COUNTER(malloc);
    TRACKED_FUNC_RESET_COUNTER(free);

    ll_list_t *list = ll_new();
    assert(list != NULL);
    assert(list->begin == NULL);
    assert(list->size == 0);

    /* ------------------------------- Push & Pop ------------------------------- */
    assert(ll_push(list, &dummy));
    assert(ll_push(list, &dummy2));
    assert(ll_pop(list)->data == &dummy2);
    assert(ll_pop(list)->data == &dummy);
    assert(ll_pop(list) == NULL);

    /* ---------------------------------- Push ---------------------------------- */
    for (i = 0; i < num_nodes; i++)
    {
        size_t *data = (size_t *)malloc(sizeof(size_t));
        *data = i;
        assert(ll_push(list, data));
        assert(list->size == i + 1);
    }

    assert(list->size == num_nodes);

    /* ----------------------------------- Get ---------------------------------- */
    for (i = 0; i < num_nodes; i++)
    {
        ll_node_t *node = ll_get(list, i);
        assert(node != NULL);
        assert(*(size_t *)node->data == i);
    }

    /* --------------------------- Get (Out-of-Bounds) -------------------------- */
    assert(ll_get(list, num_nodes) == NULL);

    /* -------------------------------- For Each -------------------------------- */
    i = 0;
    LL_FOR_EACH_NODE(list->begin, node)
    {
        assert(node != NULL);
        assert(*(size_t *)node->data == i);
        i++;
    }

    i = 0;
    LL_FOR_EACH_DATA(list->begin, data, size_t *)
    {
        assert(data != NULL);
        assert(*data == i);
        i++;
    }

    /* --------------------------- For Each (Reverse) --------------------------- */
    i = num_nodes - 1;

    LL_FOR_EACH_NODE_REVERSE(list->begin->prev, node)
    {
        assert(node != NULL);
        assert(*(size_t *)node->data == i);
        i--;
    }

    i = num_nodes - 1;
    LL_FOR_EACH_DATA_REVERSE(list->begin->prev, data2, size_t *)
    {
        assert(data2 != NULL);
        assert(*data2 == i);
        i--;
    }

    /* ------------------------------ Insert After ------------------------------ */
    ll_insert_after(list, list->begin, &dummy);
    ll_insert_after(list, list->begin, &dummy2);

    i = 0;
    LL_FOR_EACH_DATA(list->begin, data3, size_t *)
    {
        if (i == 1)
        {
            assert(*data3 == dummy2);
            break;
        }
        else if (i == 2)
        {
            assert(*data3 == dummy);
            break;
        }

        i++;
    }

    /* --------------------------------- Remove --------------------------------- */
    const size_t node_to_remove_index = 3;
    ll_node_t *node_to_remove = NULL;

    i = node_to_remove_index;
    LL_FOR_EACH_NODE(list->begin, node)
    {
        if (i == 0)
        {
            node_to_remove = node;
            break;
        }

        i--;
    }

    assert(node_to_remove != NULL);

    size_t data_arr_before[DATA_ARRAY_SIZE] = {0};
    size_t data_arr_after[DATA_ARRAY_SIZE] = {0};

    copy_list_to_array(list->begin, data_arr_before, DATA_ARRAY_SIZE);
    ll_remove(list, node_to_remove);
    copy_list_to_array(list->begin, data_arr_after, DATA_ARRAY_SIZE);

    assert(memcmp(data_arr_before, data_arr_after, node_to_remove_index * sizeof(size_t)) == 0);
    assert(memcmp((char *)data_arr_before + (node_to_remove_index + 1) * sizeof(size_t), data_arr_after + node_to_remove_index, (DATA_ARRAY_SIZE - node_to_remove_index - 1) * sizeof(size_t)) == 0);
}
