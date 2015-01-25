#include <stdtyp/set.h>
#include <stdtyp/string.h>

struct some_type {
   int a;
   char b;
   uint64_t c;
};
adt_func_pod_static(some_type);

set_gen_static(some_type_set, some_type);

set_gen_pod_static(int_set, int);
set_gen_static(string_set, string);

int
main(int argc, char **argv)
{
   create(int_set, is);
   create(string_set, ss);

   int_set_insert(&is, 4);
   int_set_insert(&is, 5);
   int_set_insert(&is, 6);
   int_set_insert(&is, 4);
   int_set_insert(&is, 4);

   adt_assert(int_set_size(&is) == 3);

   adt_assert(int_set_contains(&is, 4));
   adt_assert(int_set_contains(&is, 5));
   adt_assert(int_set_contains(&is, 6));

   adt_assert(int_set_remove(&is, 4));
   adt_assert(!int_set_remove(&is, 4));
   adt_assert(!int_set_contains(&is, 4));

   int cnt = 0;
   iter_value (int_set, &is, i) {
      adt_assert(*i == 5 || *i == 6);
      cnt++;
   }

   cnt = 0;
   iter_key (int_set, &is, i) {
      adt_assert(i == cnt);
      cnt++;
   }

   cnt = 0;
   iter (int_set, &is, item) {
      adt_assert(item.key == cnt);
      adt_assert(*item.value == 5 || *item.value == 6);
      cnt++;
   }

   adt_assert(cnt == 2);
   adt_assert(int_set_size(&is) == 2);

   adt_assert(string_set_insert(&ss, strw("apple")));
   adt_assert(string_set_insert(&ss, strw("book")));
   adt_assert(string_set_insert(&ss, strw("cat")));
   adt_assert(!string_set_insert(&ss, strw("apple")));
   adt_assert(!string_set_insert(&ss, strw("apple")));
   adt_assert(!string_set_insert(&ss, strw("apple")));

   adt_assert(string_set_size(&ss) == 3);

   create_string(diff_str, "cat");
   string_set_insert(&ss, &diff_str);

   adt_assert(string_set_size(&ss) == 3);

   adt_assert(string_set_contains(&ss, strw("cat")));
   adt_assert(string_set_contains(&ss, strw("book")));
   adt_assert(string_set_contains(&ss, strw("apple")));

   adt_assert(string_set_remove(&ss, strw("apple")));
   adt_assert(!string_set_remove(&ss, strw("apple")));

   adt_assert(!string_set_contains(&ss, strw("apple")));

   cnt = 0;
   iter_value (string_set, &ss, str) {
      adt_assert(string_equal(str, strw("book")) ||
         string_equal(str, strw("cat")));
      cnt++;
   }

   adt_assert(cnt == 2);
   adt_assert(string_set_size(&ss) == 2);

   create(some_type, a);
   a.a = 1;
   a.b = 'g';
   a.c = 8;

   create(some_type, b);
   b.a = 1;
   b.b = 'g';
   b.c = 8;

   adt_assert(some_type_equal(&a, &b));
   adt_assert(some_type_hash(&a) == some_type_hash(&b));

   create(some_type_set, sts);

   adt_assert(some_type_set_insert(&sts, &a));
   adt_assert(!some_type_set_insert(&sts, &b));

   adt_assert(some_type_set_size(&sts) == 1);

   create(int_set, res);
   for (int i = 0; i < 1000; i++)
      adt_assert(int_set_insert(&res, i));

   for (int i = 0; i < 40; i++)
      adt_assert(!int_set_insert(&res, i));

   for (int i = 0; i < 500; i++)
      adt_assert(int_set_remove(&res, i));

   for (int i = 0; i < 250; i++)
      adt_assert(int_set_insert(&res, i));

   create(int_set, res_copy);
   int_set_copy(&res_copy, &res);
}
