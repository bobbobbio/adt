// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <adt.h>
#include <stdtyp/map.h>

struct freer {
   void (*func)(void *);
};
adt_func_pod_header(freer);

struct pointer {
   void *v;
};
adt_func_pod_header(pointer);
map_gen_header(freeable_object_map, pointer, freer);

struct auto_free_pool {
   struct freeable_object_map objs;
};
adt_func_header(auto_free_pool);

void
_auto_free_pool_add_ptr(struct auto_free_pool *,
   void (*freer)(void *), void *obj);

#define auto_free_pool_add_ptr(pool, type, obj)                                \
   do {                                                                        \
      if (false)                                                               \
         type##_free(obj);                                                     \
      _auto_free_pool_add_ptr(                                                 \
         pool, (void (*)(void *))type##_free, (void *)obj);                    \
   } while (false)

#define create_ptr_in_pool(pool, type, name) \
   struct type *name = type##_new();     \
   auto_free_pool_add_ptr(pool, type, name)
