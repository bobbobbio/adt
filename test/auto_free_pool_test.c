// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <stdtyp/auto_free_pool.h>

int
main(int argc, char **argv)
{
   create(auto_free_pool, pool);

   struct string *str = string_new();
   string_append_cstring(str, "My awesome string.");
   auto_free_pool_add_ptr(&pool, string, str);

   int *p = malloc(sizeof(int));
   *p = 124;
   auto_free_pool_add_ptr(&pool, int, p);

   create_ptr_in_pool(&pool, int_vec, my_int_vec);

   int_vec_append(my_int_vec, 4);
   int_vec_append(my_int_vec, 1);
   int_vec_append(my_int_vec, 3);
   int_vec_append(my_int_vec, 2);

   int_vec_sort(my_int_vec, int_compare);
}
