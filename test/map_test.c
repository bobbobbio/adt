// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <stdtyp/map.h>
#include <stdtyp/string.h>
#include "test.h"

struct point {
   int x;
   int y;
   int z;
};
adt_func_static(point);

static void
point_init(struct point *p)
{
   p->x = 0;
   p->y = 0;
   p->z = 0;
}
static void point_destroy(struct point *p) { }

static void
point_copy(struct point *d, const struct point *s)
{
   *d = *s;
}

static
bool point_equal(struct point *a, struct point *b)
{
   return a->x == b->x && a->y == b->y && a->z == b->z;
}

map_gen_podk_static(my_map, int, point);
map_gen_static(my_map_two, string, point);

map_gen_podk_ptr_static(ptr_map, int, point);

map_gen_podk_static(char_string_map, char, string);

map_gen_podk_podv_static(int_int_map, int, int);
map_gen_podv_static(string_int_map, string, int);

void
p_inc(struct point *p)
{
   p->x++;
   p->y++;
   p->z++;
}

adt_test(map_insertion_and_iteration)
{
   create(my_map, m);
   create(my_map_two, m2);

   create(point, p);
   adt_assert(my_map_insert(&m, 0, &p));
   adt_assert(my_map_two_insert(&m2, strw("apple"), &p));
   p_inc(&p);
   adt_assert(!my_map_insert(&m, 0, &p));
   adt_assert(my_map_two_insert(&m2, strw("banana"), &p));
   p_inc(&p);
   adt_assert(my_map_insert(&m, 1, &p));
   adt_assert(my_map_two_insert(&m2, strw("cake"), &p));
   p_inc(&p);
   adt_assert(my_map_insert(&m, 2, &p));
   adt_assert(my_map_two_insert(&m2, strw("danish"), &p));
   p_inc(&p);
   adt_assert(my_map_insert(&m, 3, &p));
   adt_assert(my_map_two_insert(&m2, strw("eclair"), &p));

   iter (my_map, &m, i) {
      adt_assert(*i.key >= 0);
      adt_assert(*i.key <= 3);
      adt_assert(i.value->x == i.value->y);
      adt_assert(i.value->z == i.value->y);
   }

   iter (my_map_two, &m2, i) {
      adt_assert(string_length(i.key) > 0);
      adt_assert(i.value->x == i.value->y);
      adt_assert(i.value->z == i.value->y);
   }
}

adt_test(map_grow)
{
   create(point, p);
   create(my_map, m3);

   for (unsigned i = 0; i < 10000; i++) {
      adt_assert(my_map_insert(&m3, i, &p));
   }

   for (unsigned i = 0; i < 10000; i++) {
      create(point, po);
      my_map_get(&m3, i, &po);
      adt_assert(point_equal(&po, &p));
   }
}

adt_test(map_more_iteration)
{
   create(point, p);
   create(ptr_map, pm);
   adt_assert(ptr_map_insert(&pm, 3, &p));

   iter (ptr_map, &pm, i) {
      adt_assert(*i.key == 3);
      adt_assert(*i.value == &p);
      adt_assert(point_equal(&p, *i.value));
   }
}

adt_test(pod_key_map)
{
   create(char_string_map, cm);

   adt_assert(char_string_map_insert(&cm, 'a', strw("hello")));
   adt_assert(!char_string_map_insert(&cm, 'a', strw("hello")));

   int num = 0;
   iter (char_string_map, &cm, i) {
      num++;
      adt_assert(*i.key == 'a');
      adt_assert(string_equal(i.value, strw("hello")));
   }
   adt_assert(num == 1);
   adt_assert(char_string_map_size(&cm) == 1);
}

adt_test(pod_key_and_value_map)
{
   create(int_int_map, iim);

   adt_assert(int_int_map_insert(&iim, 4, 6));
   adt_assert(int_int_map_insert(&iim, 3, 2));
   adt_assert(!int_int_map_insert(&iim, 3, 7));

   adt_assert(*int_int_map_at(&iim, 4) == 6);
   adt_assert(*int_int_map_at(&iim, 3) == 7);

   iter (int_int_map, &iim, i) {
      adt_assert(*i.key == 4 || *i.key == 3);
      adt_assert(*i.value == 6 || *i.value == 7);
   }

   adt_assert(int_int_map_size(&iim) == 2);
}

adt_test(pod_value_map)
{
   create(string_int_map, sim);

   adt_assert(string_int_map_insert(&sim, strw("apple"), 5));
   adt_assert(string_int_map_insert(&sim, strw("bat"), 6));
   adt_assert(string_int_map_insert(&sim, strw("cat"), 7));
   adt_assert(!string_int_map_insert(&sim, strw("apple"), 8));

   iter (string_int_map, &sim, i) {
      if (string_equal(i.key, strw("apple"))) {
         adt_assert(*i.value == 8);
      } else if (string_equal(i.key, strw("bat"))) {
         adt_assert(*i.value == 6);
      } else if (string_equal(i.key, strw("cat"))) {
         adt_assert(*i.value == 7);
      } else
         panic("unknown key");
   }

   adt_assert(*string_int_map_at(&sim, strw("apple")) == 8);
   adt_assert(*string_int_map_at(&sim, strw("bat")) == 6);
   adt_assert(*string_int_map_at(&sim, strw("cat")) == 7);

   *string_int_map_at(&sim, strw("apple")) = 12;
   adt_assert(*string_int_map_at(&sim, strw("apple")) == 12);
   adt_assert(*string_int_map_at(&sim, strw("bat")) == 6);
   adt_assert(*string_int_map_at(&sim, strw("cat")) == 7);

   adt_assert(string_int_map_size(&sim) == 3);

   adt_assert(string_int_map_contains(&sim, strw("apple")));
   adt_assert(string_int_map_remove(&sim, strw("apple")));
   adt_assert(!string_int_map_remove(&sim, strw("apple")));

   adt_assert(!string_int_map_contains(&sim, strw("apple")));
   adt_assert(*string_int_map_at(&sim, strw("bat")) == 6);
   adt_assert(*string_int_map_at(&sim, strw("cat")) == 7);

   adt_assert(string_int_map_size(&sim) == 2);

   // Insert enough to go through some resizes
   create(int_int_map, res);
   for (int i = 0; i < 1000; i ++)
      adt_assert(int_int_map_insert(&res, i, 42));

   for (int i = 0; i < 500; i ++)
      adt_assert(int_int_map_remove(&res, i));

   create(int_int_map, new_res);
   int_int_map_copy(&new_res, &res);
}
