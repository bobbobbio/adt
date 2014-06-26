#include <stdtyp/map.h>
#include <stdtyp/string.h>

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

int
main(int argc, char **argv)
{
   create(my_map, m);
   create(my_map_two, m2);

   create(point, p);
   my_map_insert(&m, 0, &p);
   my_map_two_insert(&m2, strw("apple"), &p);
   p_inc(&p);
   my_map_insert(&m, 0, &p);
   my_map_two_insert(&m2, strw("banana"), &p);
   p_inc(&p);
   my_map_insert(&m, 1, &p);
   my_map_two_insert(&m2, strw("cake"), &p);
   p_inc(&p);
   my_map_insert(&m, 2, &p);
   my_map_two_insert(&m2, strw("danish"), &p);
   p_inc(&p);
   my_map_insert(&m, 3, &p);
   my_map_two_insert(&m2, strw("eclair"), &p);

   iter (my_map, &m, i) {
      assert(*i.key >= 0);
      assert(*i.key <= 3);
      assert(i.value->x == i.value->y);
      assert(i.value->z == i.value->y);
   }

   iter (my_map_two, &m2, i) {
      assert(string_length(i.key) > 0);
      assert(i.value->x == i.value->y);
      assert(i.value->z == i.value->y);
   }

   create(my_map, m3);

   for (unsigned i = 0; i < 10000; i++) {
      my_map_insert(&m3, i, &p);
   }

   for (unsigned i = 0; i < 10000; i++) {
      create(point, po);
      my_map_get(&m3, i, &po);
      assert(point_equal(&po, &p));
   }

   create(ptr_map, pm);
   ptr_map_insert(&pm, 3, &p);

   iter (ptr_map, &pm, i) {
      assert(*i.key == 3);
      assert(*i.value == &p);
      assert(point_equal(&p, *i.value));
   }

   create(char_string_map, cm);

   char_string_map_insert(&cm, 'a', strw("hello"));
   char_string_map_insert(&cm, 'a', strw("hello"));

   int num = 0;
   iter (char_string_map, &cm, i) {
      num++;
      assert(*i.key == 'a');
      assert(string_equal(i.value, strw("hello")));
   }
   assert(num == 1);
   assert(char_string_map_size(&cm) == 1);

   create(int_int_map, iim);

   int_int_map_insert(&iim, 4, 6);
   int_int_map_insert(&iim, 3, 2);
   int_int_map_insert(&iim, 3, 7);

   assert(*int_int_map_at(&iim, 4) == 6);
   assert(*int_int_map_at(&iim, 3) == 7);

   iter (int_int_map, &iim, i) {
      assert(*i.key == 4 || *i.key == 3);
      assert(*i.value == 6 || *i.value == 7);
   }

   assert(int_int_map_size(&iim) == 2);

   create(string_int_map, sim);

   string_int_map_insert(&sim, strw("apple"), 5);
   string_int_map_insert(&sim, strw("bat"), 6);
   string_int_map_insert(&sim, strw("cat"), 7);
   string_int_map_insert(&sim, strw("apple"), 8);

   iter (string_int_map, &sim, i) {
      if (string_equal(i.key, strw("apple"))) {
         assert(*i.value == 8);
      } else if (string_equal(i.key, strw("bat"))) {
         assert(*i.value == 6);
      } else if (string_equal(i.key, strw("cat"))) {
         assert(*i.value == 7);
      } else
         panic("unknown key");
   }

   assert(*string_int_map_at(&sim, strw("apple")) == 8);
   assert(*string_int_map_at(&sim, strw("bat")) == 6);
   assert(*string_int_map_at(&sim, strw("cat")) == 7);

   *string_int_map_at(&sim, strw("apple")) = 12;
   assert(*string_int_map_at(&sim, strw("apple")) == 12);
   assert(*string_int_map_at(&sim, strw("bat")) == 6);
   assert(*string_int_map_at(&sim, strw("cat")) == 7);

   assert(string_int_map_size(&sim) == 3);
}
