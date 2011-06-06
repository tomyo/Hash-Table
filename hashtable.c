/**
 * License GPLv3+
 * @file hashtable.c
 * @brief a simple hash table implementation
 * @author Ankur Shrivastava
 */
#include "hashtable.h"
#include "debug.h"
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define INITIAL_SIZE 128
#define KEY_RATIO 4

static void hash_table_element_delete_internal(hash_table_t *,
                                               hash_table_element_t *, bool);
static int hash_table_remove_internal(hash_table_t *, void *, size_t, bool);

static void hash_table_destroy_internal(hash_table_t *table, bool notify);

static unsigned int hash_table_count_keys(hash_table_t * table)
{ /* For debbuging porpuse only, use hash_table_len instead. */
    int i = 0, count = 0;
    hash_table_element_t *iter = NULL;

    for(i=0;i<table->key_num;i++)
    {
        if (table->store_house[i] != NULL)
        {
            count++;
            iter = table->store_house[i];
            while(iter->next != NULL)
            {
                count++;
                iter = iter->next;
            }
        }
    }
    return count;
}

int hash_table_len(hash_table_t *table)
{
    return table->key_count;
}

/**************************** Element Operations ******************************/
/**
 * Function to create a now hash_table element
 * @returns hash_table_element_t object when success
 * @returns NULL when no memory
 */
hash_table_element_t * hash_table_element_new()
{
    INFO("creating a new hash table element");
    return calloc(1, hash_table_element_s);
}


/**
 * Implements the common logic for the hash_table_element_delete().
 */
static void hash_table_element_delete_internal(hash_table_t * table,
                                               hash_table_element_t * element,
                                               bool notify)
{
    INFO("Deleting an hash table element");
    if (table->mode == MODE_COPY)
    {
        free(element->value);
        free(element->key);
    }
    else if (table->mode == MODE_VALUEREF)
    {
        free(element->key);
        if (notify)
        {
            if (table->value_destroy_fun)
            {
                (table->value_destroy_fun)(element->value);
            }
            free(element->key);
        }
    }
    else if (table->mode == MODE_ALLREF)
    {
        if (notify)
        { /* using destroy functions given (if any) */
            if (table->key_destroy_fun)
            {
                (table->key_destroy_fun)(element->key);
            }
            if (table->value_destroy_fun)
            {
                (table->value_destroy_fun)(element->value);
            }
        }
    }
    free(element);
}

/**
 * Function to delete an hash table element
 * @param table table from which element has to be deleted
 * @param element hash table element to be deleted
 */
void hash_table_element_delete(hash_table_t *table, hash_table_element_t *element)
{
    hash_table_element_delete_internal(table, element, true);
}


/************************** Hash Table Operations *****************************/

/**
 * Fuction to create a new hash table
 * @param mode hash_table_mode which the hash table should follow
 * @returns hash_table_t object which references the hash table
 * @returns NULL when no memory
 */
hash_table_t * hash_table_new(hash_table_mode_t mode)
{
    return hash_table_new_full(mode, NULL, NULL);
}

/**
 * Fuction to create a new hash table.
 * @param mode hash_table_mode which the hash table should follow.
 * @param key_destroy_fun to use when removing an element from hash.
 * @param value_destroy_fun to use when removing an element from hash.
 * @returns hash_table_t object which references the hash table.
 * @returns NULL when no memory.
 */
hash_table_t * hash_table_new_full(hash_table_mode_t mode,
                                   destroy_fun_t key_destroy_fun,
                                   destroy_fun_t value_destroy_fun)
{
    hash_table_t *table = NULL;

    table = calloc(1, hash_table_s);
    if (!table)
    {
        INFO("No Memory while allocating hash_table");
        return NULL;
    }
    table->mode = mode;
    table->key_destroy_fun = key_destroy_fun;
    table->value_destroy_fun = value_destroy_fun;
    table->key_num = INITIAL_SIZE;
    table->key_ratio = KEY_RATIO;
    table->store_house = (hash_table_element_t **) calloc(table->key_num, \
                          sizeof(hash_table_element_t *));
    if (!table->store_house)
    {
        INFO("No Memory while allocating hash_table store house");
        free(table);
        return NULL;
    }
    return table;
}

static void hash_table_destroy_internal(hash_table_t *table, bool notify)
{
    size_t i=0;
    assert(table != NULL);
    INFO("Deleating a hash table");
    for (;i<HASH_LEN;i++)
    {
        while (table->store_house[i])
        {
            hash_table_element_t * temp = table->store_house[i];
            table->store_house[i] = table->store_house[i]->next;
            hash_table_element_delete_internal(table, temp, notify);
            table->key_count--;
        }
        if (table->key_count == 0) break;
    }
    free(table->store_house);
    free(table);
}

void hash_table_destroy(hash_table_t *table)
{
    hash_table_destroy_internal(table, true);
}

void hash_table_free(hash_table_t *table)
{
    hash_table_destroy_internal(table, false);
}




/**
 * Function to add a key - value pair to the hash table, use HT_ADD macro
 * @param table hash table to add element to
 * @param key pointer to the key for the hash table
 * @param key_len length of the key in bytes
 * @param value pointer to the value to be added against the key
 * @param value_len length of the value in bytes
 * @returns 0 on sucess
 * @returns -1 when no memory
 */
int hash_table_add(hash_table_t * table, void * key,
                   size_t key_len, void * value, size_t value_len)
{
    size_t hash = 0;
    hash_table_element_t *element = NULL, *iter = NULL, *to_delete = NULL;
    /* Preconditions */
    assert(key != NULL);
    assert(value != NULL);
    assert(hash_table_count_keys(table) == table->key_count);

    if ((table->key_count / table->key_num) >= table->key_ratio)
    {
        LOG("Ratio(%d) reached the set limit %d\nExpanding hash_table", (table->key_count / table->key_num), table->key_ratio);
        hash_table_resize(table, table->key_num*2);
    }
    hash = HASH(key, key_len);
    element = hash_table_element_new();
    if (!element)
    {
        INFO("Cannot allocate memory for element");
        return -1; /* No Memory */
    }
    if (table->mode == MODE_COPY)
    {
        LOG("Adding a key-value pair to the hash table with hash -> %d, in COPY MODE", (int)hash);
        element->key = malloc(key_len);
        element->value = malloc(value_len);
        if (element->key && element->value)
        {
            memcpy(element->key, key, key_len);
            memcpy(element->value, value, value_len);
        }
        else
        {
            if (element->key)
            {
                free(element->key);
                INFO("Cannot allocate memory for value");
            }
            if (element->value)
            {
                free(element->value);
                INFO("Cannot allocate memory for key");
            }
            free(element);
            return -1; /* No Memory */
        }
    }
    else if (table->mode == MODE_VALUEREF)
    {
        LOG("Adding a key-value pair to the hash table with hash -> %d, in VALUEREF MODE", (int)hash);
        element->key = malloc(key_len);
        if (element->key)
        {
            memcpy(element->key, key, key_len);
        }
        else
        {
            INFO("Cannot allocate memory for key");
            free(element);
            return -1; /* No Memory */
        }
        element->value = value;
    }
    else if (table->mode == MODE_ALLREF)
    {
        LOG("Adding a key-value pair to the hash table with hash -> %d, in ALLREF MODE", (int)hash);
        element->key = key;
        element->value = value;
    }
    element->key_len = key_len;
    element->value_len = value_len;
    element->next = NULL;
    /* find the key position for chaining */
    if (!table->store_house[hash])
    {
        LOG("No Conflicts adding the first element at %d", (int)hash);
        table->store_house[hash] = element;
        table->key_count++;
    }
    else
    {
        LOG("Conflicts adding element at %d", (int)hash);
        iter = table->store_house[hash];
        while(iter->next)
        {
            while(iter->next && iter->next->key_len!=key_len)
            {
                iter = iter->next;
            }
            if(iter->next)
            {
                if (!memcmp(iter->next->key, key, key_len))
                {
                    LOG("Found Key at hash -> %d", (int)hash);
                    to_delete = iter->next;
                    iter->next = element;
                    element->next = to_delete->next;
                    hash_table_element_delete(table, to_delete);
                    /* we are replacing values, no need to change key_count */
                    return 0;
                }
                else
                {
                    iter = iter->next;
                }
            }
        }
        iter->next = element;
        table->key_count++;
    }
    return 0;
}

/**
 * Implements the common logic for the hash_table_remove() and
 * hash_table_steal() functions.
 */
static int hash_table_remove_internal(hash_table_t * table,
                               void * key, size_t key_len, bool notify)
{
    size_t hash = 0;
    hash_table_element_t *curr = NULL, *prev = NULL;
    assert(hash_table_count_keys(table) == table->key_count);

    INFO("Deleting a key-value pair from the hash table");
    if (((table->key_num/ table->key_count) >= table->key_ratio) && notify)
    {
        LOG("Ratio(%d) reached the set limit %d\nContracting hash_table",
                        (table->key_num / table->key_count), table->key_ratio);
        hash_table_resize(table, table->key_num/2);
    }
    hash = HASH(key, key_len);
    if (!table->store_house[hash])
    {
        LOG("Key Not Found -> No element at %d", (int)hash);
        return -1; /* key not found */
    }
    curr = table->store_house[hash];
    prev = curr;
    while(curr)
    {
        while(curr && curr->key_len!=key_len)
        { /* skipping keys with same hash but diferent size */
            prev = curr;
            curr = curr->next;
        }
        if(curr)
        {
            if (!memcmp(curr->key, key, key_len))
            {/* Found it */
                if (curr == table->store_house[hash])
                {
                    table->store_house[hash] = curr->next;
                }
                else
                {
                    prev->next = curr->next;
                }
                hash_table_element_delete_internal(table, curr, notify);
                INFO("Deleted a key-value pair from the hash table");
                table->key_count--;
                return 0;
            }
            prev=curr;
            curr=curr->next;
        }
    }
    INFO("Key Not Found");
    return -1; /* key not found */
}


/**
 * Function to remove a hash table element (for a given key) from a given hash table
 * @param table hash table from which element has to be removed
 * @param key pointer to the key which has to be removed
 * @param key_len size of the key in bytes
 * @returns 0 on sucess
 * @returns -1 when key is not found
 */
int hash_table_remove(hash_table_t * table, void * key, size_t key_len)
{
    return  hash_table_remove_internal(table, key, key_len, true);
}

/**
 * Like hash_table_remove, but don't call destroy functions gor key a value
 * (if given).
 */
int hash_table_steal(hash_table_t * table, void * key, size_t key_len)
{
    return  hash_table_remove_internal(table, key, key_len, false);
}


/**
 * Function to lookup a key in a particular table
 * @param table table to look key in
 * @param key pointer to key to be looked for
 * @param key_len size of the key to be searched
 * @returns NULL when key is not found in the hash table
 * @returns void* pointer to the value in the table
 */
void * hash_table_lookup(hash_table_t * table, const void * key, size_t key_len)
{
    size_t hash = HASH(key, key_len);
    hash_table_element_t *temp = NULL;
    LOG("Looking up a key-value pair for hash -> %d", (int)hash);
    if (!table->store_house[hash])
    {
        LOG("Key not found at hash %d, no entries", (int)hash);
        return NULL; /* key not found */
    }
    temp = table->store_house[hash];
    while(temp)
    {
        while(temp && temp->key_len!=key_len)
        {
            temp = temp->next;
        }
        if(temp)
        {
            if (!memcmp(temp->key, key, key_len))
            {
                LOG("Found Key at hash -> %d", (int)hash);
                return temp->value;
            }
            else
            {
                temp = temp->next;
            }
        }
    }
    LOG("Key not found at hash %d", (int)hash);
    return NULL; /* key not found */
}


/**
 * Function to lookup a key in a particular table
 * @param table table to look key in
 * @param key pointer to key to be looked for
 * @param key_len size of the key to be searched
 * @param stored_key pointer to bind the key found
 * @param stored_value pointer to bind the value found
 */
void hash_table_lookup_extended(hash_table_t * table, const void * key, size_t key_len, void ** stored_key, void ** stored_value)
{
    size_t hash = HASH(key, key_len);
    hash_table_element_t *iter = NULL;
    LOG("Looking up a key-value pair for hash -> %d", (int)hash);
    if (!table->store_house[hash])
    {
        LOG("Key not found at hash %d, no entries", (int)hash);
        return; /* key not found */
    }
    iter = table->store_house[hash];
    while(iter)
    {
        while(iter && iter->key_len!=key_len)
        {
            iter = iter->next;
        }
        if(iter)
        {
            if (!memcmp(iter->key, key, key_len))
            {
                LOG("Found Key at hash -> %d", (int)hash);
                (*stored_key) = iter->key;
                (*stored_value) = iter->value;
                return;
            }
            else
            {
                iter = iter->next;
            }
        }
    }
    LOG("Key not found at hash %d", (int)hash);
    return; /* key not found */
}

/**
 * Function to look if the exists in the hash table
 * @param key pointer to key to be looked for
 * @param key_len size of the key to be searched
 * @returns 0 when key is not found
 * @returns 1 when key is found
 */
int hash_table_has_key(hash_table_t * table, void * key, size_t key_len)
{
    size_t hash = HASH(key, key_len);
    hash_table_element_t *iter = NULL;
    LOG("Searching for key with hash -> %d", (int)hash);
    if (!table->store_house[hash])
    {
        LOG("Key not found with hash -> %d, no entries", (int)hash);
        return 0; /* key not found */
    }
    iter = table->store_house[hash];
    while(iter)
    {
        while(iter && iter->key_len!=key_len)
        {
            iter = iter->next;
        }
        if(iter)
        {
            if (!memcmp(iter->key, key, key_len))
            {
                LOG("Key Found with hash -> %d", (int)hash);
                return 1; /* key found */
            }
            iter=iter->next;
        }
    }
    LOG("Key not found with hash -> %d", (int)hash);
    return 0; /* key not found */
}


/**
 * Function that returns a hash value for a given key and key_len
 * @param key pointer to the key
 * @param key_len length of the key
 * @param max_key max value of the hash to be returned by the function
 * @returns hash value belonging to [0, max_key)
 */
uint16_t hash_table_do_hash(const void * key, size_t key_len, uint16_t max_key)
{
    const uint16_t *ptr = (const uint16_t *) key;
    uint16_t hash = 0xbabe; /* WHY NOT */
    size_t i = 0;
    for(;i<(key_len/2);i++)
    {
        hash^=(i<<4 ^ *ptr<<8 ^ *ptr);
        ptr++;
    }
    hash = hash % max_key;
    return hash;
}

/**
 * Function to resize the hash table store house
 * @param table hash table to be resized
 * @param len new length of the hash table
 * @returns -1 when no elements in hash table
 * @returns -2 when no emmory for new store house
 * @returns 0 when sucess
 */
int hash_table_resize(hash_table_t *table, size_t len)
{
    unsigned int i = 0, old_store_len = 0, count = 0;
    hash_table_mode_t mode;
    hash_table_element_t ** old_store = table->store_house;
    hash_table_element_t * etc = NULL, *next = NULL;
                        /* etc = Element To Copy */

    LOG("resizing hash table from %d to %d", table->key_num, len);

    table->store_house = calloc(len, sizeof(hash_table_element_t *));
    if (!table->store_house)
    {
        table->store_house = old_store;
        INFO("No Memory for new store house");
        return -2;
    }
    mode = table->mode;
    old_store_len = table->key_num;
    count = 0;
    table->mode = MODE_ALLREF; /* Fool new hash table to use previous values */
    table->key_num = len; /* New hash len */

    table->key_count = 0;
    for (i=0; i<old_store_len; i++)
    {
        etc = old_store[i];
        if (etc)
        {
            hash_table_add(table, etc->key, etc->key_len,
                           etc->value, etc->value_len);
            count ++;
            next = etc->next;
            free(etc);
            while(next)
            {
                etc = next;
                hash_table_add(table, etc->key, etc->key_len,
                               etc->value, etc->value_len);
                count++;
                next = etc->next;
                free(etc);
            }
        }
    }

    table->mode = mode;

    free(old_store);
    return 0;
}

/* Iter Functions */

void hash_table_iter_keys_reset(hash_table_t *self)
{
    assert(self != NULL);
    self->iter_pos = 0;
    if (self->key_count > 0)
    {
        while ((self->iter_pos <= self->key_num) &&
           (self->store_house[self->iter_pos] == NULL))
        {
            self->iter_pos++;
        }
    }

}

bool hash_table_iter_keys_is_done(hash_table_t *self)
{
    assert(self != NULL);

    return ((self->key_count == 0) || (self->iter_pos > self->key_num));
}

void *hash_table_iter_keys_next(hash_table_t *self)
{
    hash_table_element_t *current = NULL;
    static uint16_t element_index = 0;
    uint16_t i = 0;

    /* Preconditions */
    assert(self != NULL);
    assert(self->iter_pos <= self->key_num);
    assert(!hash_table_iter_keys_is_done(self));

    current = self->store_house[self->iter_pos];
    for (i=0; i < element_index; i++)
    {
        current = current->next;
    }
    if (current->next == NULL)
    {
        /* avanzamos al proximo lugar del arreglo */
        element_index = 0;
        self->iter_pos++;
        while ((self->iter_pos <= self->key_num) &&
           (self->store_house[self->iter_pos] == NULL))
        {
            self->iter_pos++;
        }
    }
    else
    {
        element_index++;
    }

    assert(current != NULL);

    return current->key;
}



