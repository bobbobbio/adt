// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <stdio.h>

#include <stdtyp/list.h>
#include "test.h"

// Create really simple type containing 3 ints
struct my_data {
   int a;
   int b;
   int c;
};
adt_func_pod_static(my_data);

static void
my_data_print(const struct my_data *m, struct string *s)
{
   string_append_format(s, "(%d, %d, %d)", m->a, m->b, m->c);
}

list_gen_static(my_list, my_data);
list_gen_pod_static(int_list, int);
list_gen_pod_static(float_list, float);
list_gen_pod_static(double_list, double);

list_gen_static(matrix, int_list);

adt_test(struct_list)
{
   create(my_list, list);

   struct my_data *one = my_data_new();
   struct my_data *two = my_data_new();
   struct my_data *three = my_data_new();
   one->a = 1;
   one->b = 2;
   one->c = 3;

   two->a = 5;
   two->b = 5;
   two->c = 5;

   three->a = 8;
   three->b = 4;
   three->c = 9;

   my_list_append(&list, one);
   my_list_append(&list, two);
   my_list_append(&list, three);
   my_data_free(one);
   my_data_free(two);
   my_data_free(three);
}

adt_test(int_list)
{
   create(int_list, ilist);

   int_list_append(&ilist, 2);
   int_list_append(&ilist, 4);
   int_list_append(&ilist, 6);
   int_list_append(&ilist, 8);
   int_list_append(&ilist, 10);
   int_list_append(&ilist, 12);
   int_list_append(&ilist, 14);
}

adt_test(float_list)
{
   create(float_list, flist);
   float_list_append(&flist, 3.1);
   float_list_append(&flist, 6.2);
   float_list_append(&flist, 9.3);
   float_list_append(&flist, 12.4);
   float_list_append(&flist, 15.5);
}

adt_test(double_list)
{
   create(double_list, dlist);
   double_list_append(&dlist, 234.23434);
   double_list_append(&dlist, 2392.9878);
   double_list_append(&dlist, 7345.537);
   double_list_append(&dlist, 7354.7534);
   double_list_append(&dlist, 73254.73);
}

adt_test(matrix)
{
   create(matrix, m);

   struct int_list *a = int_list_new();
   int_list_append(a, 1);
   int_list_append(a, 2);
   int_list_append(a, 3);
   struct int_list *b = int_list_new();
   int_list_append(b, 4);
   int_list_append(b, 5);
   int_list_append(b, 6);
   struct int_list *c = int_list_new();
   int_list_append(c, 7);
   int_list_append(c, 8);
   int_list_append(c, 9);
   matrix_append(&m, a);
   matrix_append(&m, b);
   matrix_append(&m, c);
   int_list_free(a);
   int_list_free(b);
   int_list_free(c);

   matrix_remove(&m, 0);
   matrix_remove(&m, 0);
   matrix_remove(&m, 0);
}
