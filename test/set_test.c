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

   assert(int_set_size(&is) == 3);

   assert(int_set_contains(&is, 4));
   assert(int_set_contains(&is, 5));
   assert(int_set_contains(&is, 6));

   assert(int_set_remove(&is, 4));
   assert(!int_set_remove(&is, 4));
   assert(!int_set_contains(&is, 4));

   int cnt = 0;
   iter_value (int_set, &is, i) {
      assert(*i == 5 || *i == 6);
      cnt++;
   }

   cnt = 0;
   iter_key (int_set, &is, i) {
      assert(i == cnt);
      cnt++;
   }

   cnt = 0;
   iter (int_set, &is, item) {
      assert(item.key == cnt);
      assert(*item.value == 5 || *item.value == 6);
      cnt++;
   }

   assert(cnt == 2);
   assert(int_set_size(&is) == 2);

   string_set_insert(&ss, strw("apple"));
   string_set_insert(&ss, strw("book"));
   string_set_insert(&ss, strw("cat"));
   string_set_insert(&ss, strw("apple"));
   string_set_insert(&ss, strw("apple"));
   string_set_insert(&ss, strw("apple"));

   assert(string_set_size(&ss) == 3);

   create_string(diff_str, "cat");
   string_set_insert(&ss, &diff_str);

   assert(string_set_size(&ss) == 3);

   assert(string_set_contains(&ss, strw("cat")));
   assert(string_set_contains(&ss, strw("book")));
   assert(string_set_contains(&ss, strw("apple")));

   assert(string_set_remove(&ss, strw("apple")));
   assert(!string_set_remove(&ss, strw("apple")));

   assert(!string_set_contains(&ss, strw("apple")));

   cnt = 0;
   iter_value (string_set, &ss, str) {
      assert(string_equal(str, strw("book")) ||
         string_equal(str, strw("cat")));
      cnt++;
   }

   assert(cnt == 2);
   assert(string_set_size(&ss) == 2);

   create(some_type, a);
   a.a = 1;
   a.b = 'g';
   a.c = 8;

   create(some_type, b);
   b.a = 1;
   b.b = 'g';
   b.c = 8;

   assert(some_type_equal(&a, &b));
   assert(some_type_hash(&a) == some_type_hash(&b));

   create(some_type_set, sts);

   some_type_set_insert(&sts, &a);
   some_type_set_insert(&sts, &b);

   assert(some_type_set_size(&sts) == 1);
}
