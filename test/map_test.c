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
}
