// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <stdtyp/auto_free_pool.h>

adt_func_pod_body(freer);
adt_func_pod_body(pointer);
map_gen_body(freeable_object_map, pointer, freer);
adt_func_body(auto_free_pool);

void
_auto_free_pool_add_ptr(struct auto_free_pool *pool,
   void (*freer_func)(void *), void *obj)
{
   struct freer freer = { freer_func };
   struct pointer pointer = { obj };
   bool not_replaced =
      freeable_object_map_insert(&pool->objs, &pointer, &freer);

   adt_assert(not_replaced, "Added duplicate object to auto_free_pool");
}

void
auto_free_pool_destroy(struct auto_free_pool *pool)
{
   iter (freeable_object_map, &pool->objs, entry) {
      entry.value->func(entry.key->v);
   }

   freeable_object_map_destroy(&pool->objs);
}

void
auto_free_pool_init(struct auto_free_pool *pool)
{
   freeable_object_map_init(&pool->objs);
}
