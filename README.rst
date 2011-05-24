Simple Hash Table Implementation in C
=====================================

Usefull defines when using this hash:

::

    #define KEY_SIZE sizeof(Key)
    #define VALUE_SIZE sizeof(Value)
    #define ht_create(mode) hash_table_new(mode)
    #define ht_create_full(mode, kdf, vdf) hash_table_new_full(mode, kdf, vdf)
    #define ht_lookup(table, key) hash_table_lookup(table, key, KEY_SIZE)
    #define ht_remove(table, key) hash_table_remove(table, key, KEY_SIZE)
    #define ht_insert(table, key, value) hash_table_add(table, key, KEY_SIZE, value, VALUE_SIZE)
    #define ht_destroy(table) hash_table_delete(table)
