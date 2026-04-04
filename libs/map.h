//? Collions are handled using open addressing with linear probing. When a
//? collision occurs, the next available slot in the container is used to store
//? the entry.
#if !defined(MAP_H)
#define MAP_H

#ifndef TRACKED_FUNC_CALL
#define TRACKED_FUNC_CALL(func, args_call) func args_call
#define TRACKED_FUNC_INCREMENT_COUNTER(func) ((void)0)
#endif // TRACKED_FUNC_CALL

/* -------------------------------------------------------------------------- */
/*                                  Includes                                  */
/* -------------------------------------------------------------------------- */
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------- */
/*                                    Types                                   */
/* -------------------------------------------------------------------------- */
typedef size_t (*map_hash_func_t)(const char *key);

/* -------------------------------------------------------------------------- */
/*                                   Structs                                  */
/* -------------------------------------------------------------------------- */
typedef struct
{
    char *key;
    void *value;
    size_t next_index; // Index of the next entry in case of a collision,
                       // or SIZE_MAX if there is no collision
} map_entry_t;

typedef struct
{
    size_t capacity;
    map_entry_t **container;
    map_hash_func_t hasher;
} map_t;

/* -------------------------------------------------------------------------- */
/*                                  Functions                                 */
/* -------------------------------------------------------------------------- */
/// Initializes a new map with the specified capacity and hash function.
map_t map_init(size_t capacity, map_hash_func_t hasher);

/// Initializes a new map with a default capacity and the default hash function
/// (fnv1a).
map_t map_init_default();

/// Creates a new map entry with the given key and value. The key is duplicated,
/// however, value is not duplicated, so the caller is responsible for managing
/// its memory.
map_entry_t *map_new_entry(const char *key, void *value);

/// Sets the hash function used by the map.
void map_set_hasher(map_t *map, map_hash_func_t hasher);

/// Inserts the value for the specified key in the map. If the key already exists,
/// its value is updated. Returns true if the operation was successful, or false
/// if there was an error (e.g., memory allocation failure).
bool map_set(map_t *map, const char *key, void *value);

/// Removes the entry with the specified key from the map. Returns true if the
/// entry was successfully removed, or false if the key was not found.
bool map_remove(map_t *map, const char *key);

/// Returns the value associated with the specified key, or NULL if the key is
/// not found in the map.
void *map_get(map_t *map, const char *key);

/// Frees the memory allocated for a map entry. Does not free the value stored
/// in the entry.
void map_free_entry(map_entry_t *entry);

/// Frees the memory allocated for the map and its entries. Does not free the
/// values stored in the map.
void map_free(map_t *map);

/* --------------------------------- Hashers -------------------------------- */
/// The djb2 hash function.
size_t map_hasher_djb2(const char *key);

/// The FNV-1a hash function.
size_t map_hasher_fnv1a(const char *key);

/* ----------------------------- Implementation ----------------------------- */
#ifdef MAP_IMPLEMENTATION
#undef MAP_IMPLEMENTATION

/* -------------------------------------------------------------------------- */
/*                              Public Functions                              */
/* -------------------------------------------------------------------------- */
map_t map_init(size_t capacity, map_hash_func_t hasher)
{
    return (map_t){
        .capacity = capacity,
        .container = (map_entry_t **)TRACKED_FUNC_CALL(calloc,
                                                       (capacity,
                                                        sizeof(map_entry_t *))),
        .hasher = hasher};
}

map_t map_init_default()
{
    return map_init(64, map_hasher_fnv1a);
}

map_entry_t *map_new_entry(const char *key, void *value)
{
    map_entry_t *entry = (map_entry_t *)TRACKED_FUNC_CALL(malloc, (sizeof(map_entry_t)));
    entry->key = strdup(key);
    TRACKED_FUNC_INCREMENT_COUNTER(malloc); // `strdup()` internally calls malloc.

    if (!entry->key)
    {
        TRACKED_FUNC_CALL(free, (entry));
        return NULL;
    }

    entry->value = (void *)value;
    entry->next_index = SIZE_MAX;
    return entry;
}

void map_set_hasher(map_t *map, map_hash_func_t hasher)
{
    map->hasher = hasher;
}

bool map_set(map_t *map, const char *key, void *value)
{
    size_t lookup_index = map->hasher(key) % map->capacity;

    // Insert the new entry directly if the slot is empty.
    if (map->container[lookup_index] == NULL)
    {
        map->container[lookup_index] = map_new_entry(key, value);
        return true;
    }

    // Keep track of the previous index and entry to handle collisions.
    map_entry_t *prev_entry = NULL;

    // Traverse the collision chain to find the correct slot for the new entry.
    while (map->container[lookup_index] != NULL)
    {
        map_entry_t *entry = map->container[lookup_index];

        // If the key already exists, update its value and return.
        if (strcmp(entry->key, key) == 0)
        {
            entry->value = value;
            return true;
        }

        // Collision chain ended and no matching key was found.
        // Break to find the next available slot.
        if ((prev_entry = entry)->next_index == SIZE_MAX)
            break;

        lookup_index = entry->next_index;
    }

    // Find the next available slot for the new entry.
    size_t prev_index = lookup_index;
    lookup_index = (lookup_index + 1) % map->capacity;

    while (map->container[lookup_index] != NULL)
    {
        // The map is full, cannot insert the new entry.
        if (lookup_index == prev_index)
            return false;

        lookup_index = (lookup_index + 1) % map->capacity;
    }

    // Insert the new entry at the found slot and update the previous entry
    // to point to the new entry.
    map->container[lookup_index] = map_new_entry(key, value);
    prev_entry->next_index = lookup_index;

    return true;
}

bool map_remove(map_t *map, const char *key)
{
    size_t lookup_index = map->hasher(key) % map->capacity;

    while (map->container[lookup_index] != NULL)
    {
        map_entry_t *entry = map->container[lookup_index];

        if (strcmp(entry->key, key) == 0)
        {
            map_free_entry(entry);
            map->container[lookup_index] = NULL;

            return true;
        }

        if (entry->next_index == SIZE_MAX)
            break;

        lookup_index = entry->next_index;
    }

    return false;
}

void *map_get(map_t *map, const char *key)
{
    size_t lookup_index = map->hasher(key) % map->capacity;

    while (map->container[lookup_index] != NULL)
    {
        map_entry_t *entry = map->container[lookup_index];

        if (strcmp(entry->key, key) == 0)
            return entry->value;

        if (entry->next_index == SIZE_MAX)
            break;

        lookup_index = entry->next_index;
    }

    return NULL;
}

void map_free_entry(map_entry_t *entry)
{
    TRACKED_FUNC_CALL(free, (entry->key));
    TRACKED_FUNC_CALL(free, (entry));
}

void map_free(map_t *map)
{
    for (size_t i = 0; i < map->capacity; i++)
    {
        if (!map->container[i])
            continue;

        map_free_entry(map->container[i]);
    }

    TRACKED_FUNC_CALL(free, (map->container));
    map->container = NULL;
    map->capacity = 0;
}

size_t map_hasher_djb2(const char *key)
{
    size_t hash = 5381;
    char c;

    while ((c = *key++))
        hash = ((hash << 5) + hash) + (uint8_t)c; /* hash * 33 + c */

    return hash;
}

size_t map_hasher_fnv1a(const char *key)
{
    size_t hash = 0xcbf29ce484222325;
    char c;

    while ((c = *key++))
    {
        hash ^= (uint8_t)c;
        hash *= 0x100000001b3;
    }

    return hash;
}

#endif // MAP_IMPLEMENTATION
#endif // MAP_H
