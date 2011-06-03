Simple Hash Table Implementation in C
=====================================

Usefull macros for using this hash (hashtable.h):

::
    
    #define ht_new(mode) hash_table_new(mode)
    #define ht_new_full(mode, kdf, vdf) hash_table_new_full(mode, kdf, vdf)
    #define ht_lookup(table, key) hash_table_lookup(table, key, sizeof(*key))
    #define ht_lookup_extended(table, key, store_key, store_value) \
    hash_table_lookup_extended(table, key, sizeof(*key), store_key, store_value)
    #define ht_steal(table, key) hash_table_steal(table, key, sizeof(*key))
    #define ht_remove(table, key) hash_table_remove(table, key, sizeof(*key))
    #define ht_insert(table, key, value) \
    hash_table_add(table, key, sizeof(*key), value, sizeof(*value))
    #define ht_destroy(table) hash_table_delete(table)
