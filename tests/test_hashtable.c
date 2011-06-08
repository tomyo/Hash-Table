#include <check.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <assert.h>

#include "hashtable.h"
#include "test_hashtable.h"

hash_table_t *net = NULL; /* tmp hash */

/* Precondiciones */
START_TEST(test_hash_table_destroy_null)
{
    ht_destroy(NULL);
}
END_TEST

START_TEST(test_hash_table_insert_null)
{
    hash_table_t *table = hash_table_new(MODE_ALLREF);
    hash_table_add(table, NULL, 0, NULL, 0);
}
END_TEST

/* Crear y destruir */
START_TEST(test_hash_table_new_destroy)
{
    hash_table_t *table = hash_table_new(MODE_ALLREF);
    ht_destroy(table);
}
END_TEST

/* Testeos de funcionalidad */

START_TEST(test_hash_table_basic_consistency)
{
    hash_table_t *table = hash_table_new(MODE_ALLREF);
    int k[] = {0,1,2,3,4,5,6,7};
    int v = 42 , i = 0;
    char test[5] = "hola";
    for (i = 0; i < 8; i ++)
    {
        ht_insert(table, &k[i], &v);
    }
    fail_unless(ht_has_key(table, &k[1]));
    fail_unless(*(int *)ht_lookup(table, &k[1]) == 42);
    ht_replace(table, &k[1], test);
    fail_unless(memcmp(ht_lookup(table, &k[1]), test, 5) == 0);
    fail_unless(ht_has_key(table, &k[1]));
    ht_destroy(table);
}
END_TEST

START_TEST(test_hash_table_keys_iter_basic)
{
    hash_table_t *table = hash_table_new(MODE_ALLREF);
    int *current = NULL;
    int k[] = {0,1,2,3,4,5,6,7};
    int v = 42;
    int i = 0;
    for (i = 0; i < 8; i ++)
    {
        ht_insert(table, &k[i], &v);
    }

    ht_iter_keys_init(table);
    while (!ht_iter_keys_is_done(table))
    {
        current = ht_iter_keys_next(table);
        fail_unless((0 <= *((int *)current)) &&
                    (7 >= *((int *)current)));
    }

    ht_destroy(table);
}
END_TEST

START_TEST(test_hash_table_keys_iter_full)
{
    hash_table_t *table = hash_table_new(MODE_ALLREF);
    int k[2048], k_chk[2048];
    int v = 42;
    int i = 0, current = 0;

    for (i = 0; i < 2048; i++)
    {
        k[i] = i;
        ht_insert(table, &k[i], &v);
    }

    ht_iter_keys_init(table);
    while (!ht_iter_keys_is_done(table))
    {
        current = *((int *)ht_iter_keys_next(table));
        fail_unless((current <= 2048) && (current >= 0));
        k_chk[current] = 1;
    }
    for (i=0; i< 2048; i++)
    {
        fail_unless(k_chk[i] == 1);
    }

    ht_destroy(table);
}
END_TEST


/* Armado de la test suite */
Suite *hash_table_suite(void){
    Suite *s = suite_create("hash_table");
    TCase *tc_preconditions = tcase_create("Precondition");
    TCase *tc_creation = tcase_create("Creation");
    TCase *tc_functionality = tcase_create("Functionality");

    /* Precondiciones */
    tcase_add_test_raise_signal(tc_preconditions, test_hash_table_destroy_null, SIGABRT);
    tcase_add_test_raise_signal(tc_preconditions, test_hash_table_insert_null, SIGABRT);
    suite_add_tcase(s, tc_preconditions);

    /* Creation */
    tcase_add_test(tc_creation, test_hash_table_new_destroy);
    suite_add_tcase(s, tc_creation);

    /* Funcionalidad */
    tcase_add_test(tc_functionality, test_hash_table_basic_consistency);
    tcase_add_test(tc_functionality, test_hash_table_keys_iter_basic);
    tcase_add_test(tc_functionality, test_hash_table_keys_iter_full);
    suite_add_tcase(s, tc_functionality);

    return s;
}

/* Memory Test */
void hash_table_memory_test(void){
    /* This code should have no memory leaks */


}
