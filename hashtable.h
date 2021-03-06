/**
 * License GPLv3+
 * @file hashtable.h
 * @brief a simple hash table implementation
 * @author Ankur Shrivastava
 */
#ifndef _HASHTABLE_H
#define _HASHTABLE_H

#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>

#define HASH_LEN table->key_num
#define HASH(x,y) hash_table_do_hash(x,y,HASH_LEN)


#define ht_new(mode) hash_table_new(mode)
#define ht_new_full(mode, kdf, vdf) hash_table_new_full(mode, kdf, vdf)
#define ht_lookup(table, key) hash_table_lookup(table, key, sizeof(*key))
#define ht_lookup_extended(table, key, store_key, store_value) \
hash_table_lookup_extended(table, key, sizeof(*key), store_key, store_value)
#define ht_steal(table, key) hash_table_steal(table, key, sizeof(*key))
#define ht_remove(table, key) hash_table_remove(table, key, sizeof(*key))
#define ht_insert(table, key, value) \
hash_table_add(table, key, sizeof(*key), value, sizeof(*value))
#define ht_replace(table, key, value) \
hash_table_replace(table, key, sizeof(*key), value, sizeof(*value))
#define ht_has_key(table, key) hash_table_has_key(table, key, sizeof(*key))
#define ht_destroy(table) hash_table_destroy(table)
#define ht_free(table) hash_table_free(table);
#define ht_iter_keys_init(self) hash_table_iter_keys_init(self)
#define ht_iter_keys_next(self) hash_table_iter_keys_next(self)
#define ht_iter_keys_is_done(self) hash_table_iter_keys_is_done(self)

typedef void (*destroy_fun_t) (void *);

/* forward declaration */
typedef struct hash_table_element hash_table_element_t;

/**
 * @struct hash_table_element "hashtable.h"
 * @brief stores an hash table element for use in the hash table
 */
struct hash_table_element
{
    /**
     * store the length in bytes of the key
     */
    size_t key_len;
    /**
     * stores the length in bytes of the key (only for copy mode)
     */
    size_t value_len;
    /**
     * pointer to the key
     */
    void * key;
    /**
     * pointer to the value
     */
    void * value;
    /**
     * next chained key for this hash
     */
    hash_table_element_t * next;
};
#define hash_table_element_s sizeof(hash_table_element_t)

/**
 * @enum hash_table_mode defines the mode of operation of hash table
 */
typedef enum hash_table_mode{
    /** copy mode here values as well as key is copied */
    MODE_COPY,
    /** value reference mode, here ONLY key is copies and value is always referred */
    MODE_VALUEREF,
    /** in this mode all keys and values are referred */
    MODE_ALLREF
} hash_table_mode_t;

/**
 * @struct hash_table "hashtable.h"
 * @brief identifies the hashtable for which operations are to be performed
 */
typedef struct hash_table
{
    /**
     * the hash table array where all values are stored
     */
    hash_table_element_t  ** store_house;

    /**
     * mode of the hash table
     */
    hash_table_mode_t mode;

    /**
     * key destroy funcion
     */
    destroy_fun_t key_destroy_fun;

    /**
     * value destroy funcion
     */
    destroy_fun_t value_destroy_fun;

    /**
     * number of keys in the hash table
     */
    size_t key_count;

    /**
     * number of keys allocated in the hash table
     */
    uint16_t key_num;

    /**
     * the ratio of key_count / key_num at which the hash table should be expanded
     */
    size_t key_ratio;

    /* internal reference to iter pos in hash */
    uint16_t iter_pos;

} hash_table_t;
#define hash_table_s sizeof(hash_table_t)

/* Iter Operations */

void hash_table_iter_keys_init(hash_table_t *);

/* Dont iter while adding new elements to the hash (it could resize)*/
void *hash_table_iter_keys_next(hash_table_t *);

bool hash_table_iter_keys_is_done(hash_table_t *);

/* element operations */

/**
 * Function to create a now hash_table element
 * @returns hash_table_element_t object when success
 * @returns NULL when no memory
 */
hash_table_element_t * hash_table_element_new(void);

/**
 * Function to delete an hash table element
 * @param table table from which element has to be deleted
 * @param element hash table element to be deleted
 */

void hash_table_element_delete(hash_table_t *, hash_table_element_t *);

/**
 * Function that returns a hash value for a given key and key_len
 * @param key pointer to the key
 * @param key_len length of the key
 * @param max_key max value of the hash to be returned by the function
 * @returns hash value belonging to [0, max_key)
 */
uint16_t hash_table_do_hash(const void * key, size_t key_len, uint16_t max_key);

/* hash table operations */

/**
 * Fuction to create a new hash table
 * @param mode hash_table_mode which the hash table should follow
 * @returns hash_table_t object which references the hash table
 * @returns NULL when no memory
 */
hash_table_t * hash_table_new(hash_table_mode_t);

hash_table_t * hash_table_new_full(hash_table_mode_t, destroy_fun_t, destroy_fun_t);


/**
 * Function to destroy the hash table, if table was created with full mode
 * destroing functions will be call for every key-value in the hash. If you
 * don't want that, use hash_table_free instead wich only frees the hash. 
 * @param table hash table to be destroy
 */
void hash_table_destroy(hash_table_t *);


/**
 * Function to destroy the hash table, keys and values must be destroy by
 * user (if MODE use references). Otherwise use hash_table_destroy.
 * @param table hash table to be destroy
 */
void hash_table_free(hash_table_t *);


/**
 * macro to add a key - value pair to the hash table
 * @note use this macro when size of key and/or value can be given by sizeof
 * @param table hash table to add element to
 * @param key pointer to the key for the hash table
 * @param value pointer to the value to be added against the key
 * @returns 0 on sucess
 * @returns -1 when no memory
 */
#define HT_ADD(table, key, value) hash_table_add(table, (void *) key, sizeof(*key), (void *) value, sizeof(*value))

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
int hash_table_add(hash_table_t *, void *, size_t, void *, size_t);


/* Replace an existing value of the hash without calling the destroy
 * function given at hash_table_new_full. otherwise behaves just like add.
 * Precondition: Key must have an existing entry in the table.
 */
int hash_table_replace(hash_table_t *, void *, size_t, void *, size_t);


int hash_table_len(hash_table_t *);
/**
 * macro to remove an hash table element (for a given key) from a given hash table
 * @note use this macro when size of key and/or value can be given by sizeof
 * @param table hash table from which element has to be removed
 * @param key pointer to the key which has to be removed
 * @returns 0 on sucess
 * @returns -1 when key is not found
 */
#define HT_REMOVE(table, key) hash_table_remove(table, key, sizeof(*key))

/**
 * Function to remove an hash table element (for a given key) from a given hash table
 * @param table hash table from which element has to be removed
 * @param key pointer to the key which has to be removed
 * @param key_len size of the key in bytes
 * @returns 0 on sucess
 * @returns -1 when key is not found
 */
int hash_table_remove(hash_table_t *, void *, size_t);

/**
 * Like hash_table_remove, but don't call destroy functions gor key a value
 * (if given).
 */
int hash_table_steal(hash_table_t *, void *, size_t);


/**
 * macro to lookup a key in a particular table
 * @param table table to look key in
 * @param key pointer to key to be looked for
 * @returns NULL when key is not found in the hash table
 * @returns void* pointer to the value in the table
 */
#define HT_LOOKUP(table, key) hash_table_lookup(table, key, sizeof(*key))

/**
 * Function to lookup a key in a particular table
 * @note use this macro when size of key and/or value can be given by sizeof
 * @param table table to look key in
 * @param key pointer to key to be looked for
 * @param key_len size of the key to be searched
 * @returns NULL when key is not found in the hash table
 * @returns void* pointer to the value in the table
 */
void * hash_table_lookup(hash_table_t *, const void *, size_t);

#define HT_LOOKUP_EXTENDED(table, key, stored_key, stored_value) \
hash_table_lookup_extended(table, key, sizeof(*key), stores_key, stored_value)

/**
 * Function to lookup a key in a particular table
 * @param table table to look key in
 * @param key pointer to key to be looked for
 * @param key_len size of the key to be searched
 * @param stored_key pointer to bind the key found
 * @param stored_value pointer to bind the value found
 */
void hash_table_lookup_extended(hash_table_t * table, const void * key,
        size_t key_len, void ** stored_key, void ** stored_value);

/**
 * macro to look if the exists in the hash table
 * @note use this macro when size of key and/or value can be given by sizeof
 * @param key pointer to key to be looked for
 * @returns 0 when key is not found
 * @returns 1 when key is found
 */
#define HT_HAS_KEY(table, key) hash_table_has_key(table, key, sizeof(*key))

/**
 * Function to look if the exists in the hash table
 * @param key pointer to key to be looked for
 * @param key_len size of the key to be searched
 * @returns 0 when key is not found
 * @returns 1 when key is found
 */
int hash_table_has_key(hash_table_t *, void *, size_t);

/**
 * Function to resize the hash table store house
 * @param table hash table to be resized
 * @param len new length of the hash table
 * @returns -1 when no elements in hash table
 * @returns -2 when no emmory for new store house
 * @returns 0 when sucess
 */
int hash_table_resize(hash_table_t *, size_t);

#endif
