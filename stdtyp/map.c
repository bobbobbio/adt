// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <stdtyp/map.h>
#include <stdtyp/string.h>

#include <string.h>
#include <time.h>

#define MAP_START_SIZE (100)
// Resize the table when it is this percent full or more
#define MAP_RESIZE_FACTOR 3 / 4

struct map_table_item {
   void *key;
   void *data;
   bool deleted;
};
adt_func_pod_static(map_table_item);

void
map_table_item_print(const struct map_table_item *item, struct string *s)
{
   string_append_format(s, "map_table_item(%p, %p, %s)", item->key, item->data,
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
      adt_assert(map_table_at(m->buckets, i)->key == NULL);
      adt_assert(map_table_at(m->buckets, i)->data == NULL);
      adt_assert(map_table_at(m->buckets, i)->deleted == false);
   }
}

void
map_destroy(struct map *m)
{
   map_table_free(m->buckets);
}

static uint64_t
_map_nbuckets(const struct map *m)
{
   uint64_t s = map_table_size(m->buckets);
   adt_assert(s > 0);
   return s;
}

void
_map_resize(struct map *m)
{
   uint64_t old_size a_unused = m->size;
   struct map_table *old_buckets = m->buckets;
   uint64_t size = map_table_size(m->buckets) * 2;
   m->buckets = map_table_new_size(size);
   m->size = 0;
   // rehash everything
   for (uint64_t i = 0; i < map_table_size(old_buckets); i++) {
      struct map_table_item *item = map_table_at(old_buckets, i);
      if (!item->deleted && item->key != NULL)
         map_insert(m, item->key, item->data, NULL, NULL);
   }
   adt_assert(m->size < _map_nbuckets(m));
   adt_assert(m->size == old_size);

   map_table_free(old_buckets);
}

static void
_map_resize_check(struct map *m)
{
   if (m->size >= _map_nbuckets(m) * MAP_RESIZE_FACTOR) {
      _map_resize(m);
      adt_assert(m->size < _map_nbuckets(m) * MAP_RESIZE_FACTOR);
   }
}

static struct map_table_item *
_map_get(const struct map *m, const void *k)
{
   adt_assert(k != NULL);

   uint64_t l = m->hash(k) % _map_nbuckets(m);

   uint64_t not_seen = _map_nbuckets(m);
   while (not_seen > 0) {
      struct map_table_item *item = map_table_at(m->buckets, l);
      if (item->key == NULL)
         return NULL;
      if (!item->deleted && m->key_compare(item->key, k) == 0) {
         return item;
      }
      l = (l + 1) % _map_nbuckets(m);

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
   adt_assert(k != NULL);

   // Set to NULL if we replace nothing
   if (ko != NULL)
      *ko = NULL;
   if (vo != NULL)
      *vo = NULL;

   _map_resize_check(m); // this ensures our map has room

   uint64_t l = m->hash(k) % _map_nbuckets(m);

   bool inserted = false;
   uint64_t not_seen = _map_nbuckets(m);
   while (not_seen > 0) {
      struct map_table_item *item = map_table_at(m->buckets, l);
      // As soon as we see a NULL key we can finish
      if (item->key == NULL) {
         if (!inserted) {
            item->deleted = false;
            item->key = k;
            item->data = v;
            m->size++;
            inserted = true;
         }
         return;
      }
      // We can use deleted entries
      else if (item->deleted) {
         if (!inserted) {
            item->deleted = false;
            item->key = k;
            item->data = v;
            m->size++;
            inserted = true;
         }
      }
      // If we find a bucket with the same value
      else if (m->key_compare(item->key, k) == 0) {
         // If we are replacing something, return out the value
         if (item->key != NULL && !item->deleted) {
            // We can't return two things out at once
            adt_assert(*ko == NULL);
            adt_assert(*vo == NULL);
            if (ko != NULL)
               *ko = item->key;
            if (vo != NULL)
               *vo = item->data;
            m->size--;
         }
         if (!inserted) {
            item->deleted = false;
            item->key = k;
            item->data = v;
            m->size++;
            inserted = true;
         } else {
            // If we have inserted something, and found a repeat we have to
            // delete it
            item->deleted = true;
         }
      }
      l = (l + 1) % _map_nbuckets(m);

      not_seen--;
   }
   adt_assert(inserted,
      "Map is totally broken, somehow didn't find room for item");
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

void
map_get_random(const struct map *m, void **k_out, void **v_out)
{
   adt_assert(m->size > 0);
   uint64_t c_index = rand() % m->size;

   uint64_t index = 0;
   uint64_t t_index = 0;
   while (t_index < _map_nbuckets(m)) {
      struct map_table_item *item = map_table_at(m->buckets, t_index);
      if (item->key != NULL && !item->deleted) {
         if (index == c_index) {
            if (k_out != NULL)
               *k_out = item->key;
            if (v_out != NULL)
               *v_out = item->data;
            return;
         }
         index++;
      }
      t_index++;
   }
   panic("Map totally broke");
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

