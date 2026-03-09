//? This file contains implementation of a circular doubly linked list.
#if !defined(LINKED_LIST_H)
#define LINKED_LIST_H

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
#define LL_FOR_EACH_NODE_IMPL(list, var, attr)                                 \
    for (ll_node_t *var = (list), *ll_foreach_start__ = (list); (var) != NULL; \
         (var) = ((var)->attr == ll_foreach_start__ ? NULL : (var)->attr))

#define LL_FOR_EACH_DATA_IMPL(list, var, type, attr)                          \
    type var = NULL;                                                          \
                                                                              \
    for (ll_node_t *ll_foreach_node__ = (list), *ll_foreach_start__ = (list); \
         ll_foreach_node__ != NULL &&                                         \
         (((var) = (type)ll_foreach_node__->data), 1);                        \
         ll_foreach_node__ = ((ll_foreach_node__->attr == ll_foreach_start__) ? NULL : ll_foreach_node__->attr))

#define LL_FOR_EACH_NODE(list, var) \
    LL_FOR_EACH_NODE_IMPL(list, var, next)

#define LL_FOR_EACH_DATA(list, var, type) \
    LL_FOR_EACH_DATA_IMPL(list, var, type, next)

#define LL_FOR_EACH_NODE_REVERSE(list, var) \
    LL_FOR_EACH_NODE_IMPL(list, var, prev)

#define LL_FOR_EACH_DATA_REVERSE(list, var, type) \
    LL_FOR_EACH_DATA_IMPL(list, var, type, prev)

/* -------------------------------------------------------------------------- */
/*                                   Structs                                  */
/* -------------------------------------------------------------------------- */
typedef struct ll_node_s
{
    void *data;
    struct ll_node_s *next;
    struct ll_node_s *prev;
} ll_node_t;

typedef struct
{
    ll_node_t *begin;
    size_t size;
} ll_list_t;

/* -------------------------------------------------------------------------- */
/*                                  Functions                                 */
/* -------------------------------------------------------------------------- */
/// Creates a new linked list and returns a pointer to it. Returns NULL if memory allocation fails.
ll_list_t *ll_new();

/// Creates a new linked list node with the given data and returns a pointer to it. Returns NULL if memory allocation fails.
ll_node_t *ll_new_node(void *data);

/// Frees the specified node and optionally its data.
void ll_free_node(ll_node_t *node, bool free_data);

/// Retrieves the node at the specified index in the linked list. Returns NULL if the index is out of bounds.
ll_node_t *ll_get(ll_list_t *list, size_t index);

/// Inserts a new node with the given data at the end of the linked list. Returns true if the insertion was successful, false otherwise.
bool ll_push(ll_list_t *list, void *data);

/// Pops the last node from the linked list and returns its data. Returns NULL if the list is empty or list only contains the head.
ll_node_t *ll_pop(ll_list_t *list);

/// Inserts a new node with the given data after the specified node in the linked list. Returns true if the insertion was successful, false otherwise.
bool ll_insert_after(ll_list_t *list, ll_node_t *node, void *data);

/// Removes the specified node from the linked list.
void ll_remove(ll_list_t *list, ll_node_t *node);

void ll_print_nodes(ll_list_t *list);

/* ----------------------------- Implementation ----------------------------- */
#ifdef LINKED_LIST_IMPLEMENTATION
#undef LINKED_LIST_IMPLEMENTATION

/* -------------------------------------------------------------------------- */
/*                              Private Functions                             */
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
/*                              Public Functions                              */
/* -------------------------------------------------------------------------- */
ll_list_t *ll_new()
{
    ll_list_t *list = (ll_list_t *)TRACKED_FUNC_CALL(malloc, (sizeof(ll_list_t)));
    if (list == NULL)
        return NULL;

    list->begin = NULL;
    list->size = 0;

    return list;
}

ll_node_t *ll_new_node(void *data)
{
    ll_node_t *node = (ll_node_t *)TRACKED_FUNC_CALL(malloc, (sizeof(ll_node_t)));
    if (node == NULL)
        return NULL;

    node->data = data;
    node->next = NULL;
    node->prev = node;

    return node;
}

void ll_free_node(ll_node_t *node, bool free_data)
{
    if (free_data)
        TRACKED_FUNC_CALL(free, (node->data));

    TRACKED_FUNC_CALL(free, (node));
}

ll_node_t *ll_get(ll_list_t *list, size_t index)
{
    if (index >= list->size)
        return NULL;

    LL_FOR_EACH_NODE(list->begin, node)
    {
        if (index == 0)
            return node;

        index--;
    }

    return NULL;
}

bool ll_push(ll_list_t *list, void *data)
{
    // List is empty, create the first node and set it as the beginning of the list.
    if (list->begin == NULL)
    {
        list->begin = ll_new_node(data);
        if (list->begin == NULL)
            return false;

        list->begin->next = list->begin;
        list->begin->prev = list->begin;
        list->size = 1;
        return true;
    }

    ll_node_t *node = list->begin->prev;

    if (!ll_insert_after(list, node, data))
        return false;

    list->begin->prev = node->next;
    return true;
}

ll_node_t *ll_pop(ll_list_t *list)
{
    if (list->size == 0)
        return NULL;

    ll_node_t *node = list->begin->prev;
    ll_remove(list, node);

    return node;
}

bool ll_insert_after(ll_list_t *list, ll_node_t *node, void *data)
{
    ll_node_t *new_node = ll_new_node(data);
    if (new_node == NULL)
        return false;

    new_node->next = node->next;
    node->next = new_node;

    new_node->next->prev = new_node;
    new_node->prev = node;

    list->size++;
    return true;
}

void ll_remove(ll_list_t *list, ll_node_t *node)
{
    // `node` is the only node in the list.
    if (node == list->begin && node->next == node)
    {
        list->begin = NULL;
        goto exit;
    }

    node->prev->next = node->next;
    node->next->prev = node->prev;

    if (node == list->begin)
        list->begin = node->next;

exit:
    list->size--;
}

void ll_print_nodes(ll_list_t *list)
{
    size_t i = 0;
    LL_FOR_EACH_NODE(list->begin, node)
    {
        if (i != 0)
            printf("-> ");

        printf("0x%p ", node->data);
        i++;
    }

    printf("\n");
}


#endif // LINKED_LIST_IMPLEMENTATION
#endif // LINKED_LIST_H
