#include <stdtyp/map.h>
#include <stdtyp/string.h>

#include <string.h>

#define MAP_START_SIZE (100)
// Resize the table when it is this percent full or more
#define MAP_RESIZE_FACTOR (0.75)

struct map_table_item {
   void *key;
   void *data;
   bool deleted;
};
adt_func_pod_static(map_table_item);

void
map_table_item_print(const struct map_table_item *item)
{
   printf("map_table_item(%p, %p, %s)", item->key, item->data,
      item->deleted ? "deleted=true" : "deleted=false");
}

vector_gen_static(map_table, map_table_item);

adt_func_body(map);

void
map_init(struct map *m)
{
   m->buckets = map_table_new_size(MAP_START_SIZE);
   m->key_compare = NULL;
   m->size = 0;
   m->hash = NULL;

   for (unsigned i = 0; i < MAP_START_SIZE; i++) {
      assert(map_table_at(m->buckets, i)->key == NULL);
      assert(map_table_at(m->buckets, i)->data == NULL);
      assert(map_table_at(m->buckets, i)->deleted == false);
   }
}

void
map_destroy(struct map *m)
{
   map_table_free(m->buckets);
}

void
_map_resize(struct map *m)
{
   struct map_table *old_buckets = m->buckets;
   uint64_t size = map_table_size(m->buckets) * 2;
   m->buckets = map_table_new_size(size);
   // rehash everything
   for (uint64_t i = 0; i < map_table_size(old_buckets); i++) {
      struct map_table_item *item = map_table_at(old_buckets, i);
      if (!item->deleted && item->key != NULL)
         map_insert(m, item->key, item->data, NULL, NULL);
   }

   map_table_free(old_buckets);
}

static uint64_t
_map_nbuckets(const struct map *m)
{
   uint64_t s = map_table_size(m->buckets);
   assert(s > 0);
   return s;
}

static void
_map_resize_check(struct map *m)
{
   if (m->size >= _map_nbuckets(m) * MAP_RESIZE_FACTOR) {
      _map_resize(m);
   }
}

static struct map_table_item *
_map_get(const struct map *m, const void *k)
{
   assert(k != NULL);

   uint64_t l = m->hash(k) % _map_nbuckets(m);

   uint64_t not_seen = _map_nbuckets(m);
   while (not_seen > 0) {
      struct map_table_item *item = map_table_at(m->buckets, l);
      if (item->key == NULL)
         return NULL;
      if (m->key_compare(item->key, k) == 0) {
         if (!item->deleted)
            return item;
         else
            return NULL;
      }
      l = l + 1 % _map_nbuckets(m);

      not_seen--;
   }

   return NULL;
}

void **
map_at(const struct map *m, const void *k)
{
   struct map_table_item *item = _map_get(m, k);

   if (item == NULL)
      return NULL;

   return &item->data;
}

void *
map_get(const struct map *m, const void *k)
{
   void **v = map_at(m, k);

   if (v == NULL)
      return NULL;

   return *v;
}

void
map_insert(struct map *m, void *k, void *v, void **ko, void **vo)
{
   assert(k != NULL);

   _map_resize_check(m); // this ensures our map has room

   uint64_t l = m->hash(k) % _map_nbuckets(m);

   uint64_t not_seen = _map_nbuckets(m);
   while (not_seen > 0) {
      struct map_table_item *item = map_table_at(m->buckets, l);
      if (item->key == NULL || m->key_compare(item->key, k) == 0) {
         if (item->key != NULL && item->deleted == false) {
            if (ko != NULL)
               *ko = item->key;
            if (vo != NULL)
               *vo = item->data;
            m->size--;
         }
         item->deleted = false;
         item->key = k;
         item->data = v;
         m->size++;
         return;
      }
      l = l + 1 % _map_nbuckets(m);

      not_seen--;
   }
   panic("Map is totally broken, somehow didn't find room for item");
}

bool
map_remove(struct map *m, const void *k, void **k_out, void **v_out)
{
   struct map_table_item *item = _map_get(m, k);
   if (item == NULL)
      return false;

   item->deleted = true;
   *v_out = item->data;
   *k_out = item->key;

   m->size--;

   return true;
}

uint64_t
map_size(const struct map *m)
{
   return m->size;
}

bool
map_contains(const struct map *m, const void *k)
{
   struct map_table_item *item = _map_get(m, k);

   return item != NULL;
}

void
map_clear(struct map *m)
{
   struct map_table *old_buckets = m->buckets;
   uint64_t size = map_table_size(m->buckets) * 2;
   m->buckets = map_table_new_size(size);

   map_table_free(old_buckets);
}

bool
map_iterate(const struct map *m, struct aiter *p, void **k, void **v)
{
   if (p->ipos >= _map_nbuckets(m))
      return false;

   while (map_table_at(m->buckets, p->ipos)->key == NULL ||
          map_table_at(m->buckets, p->ipos)->deleted) {
      p->ipos++;
      if (p->ipos >= _map_nbuckets(m))
         return false;
   }

   *k = map_table_at(m->buckets, p->ipos)->key;
   *v = map_table_at(m->buckets, p->ipos)->data;

   p->ipos++;

   return true;
}

