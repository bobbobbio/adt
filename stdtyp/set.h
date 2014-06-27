#include <adt.h>
#include <stdtyp/vector.h>
#include <stdtyp/iter.h>
#include <stdtyp/map.h>

#define _set_gen_header(name, ktype, ktype_ref, f) \
   struct name { struct map map; }; \
   _adt_func_header(name, f); \
   _gen_iter_header(name, int, ktype_ref, f); \
   f void name##_insert(struct name *, ktype); \
   f bool name##_remove(struct name *, ktype); \
   f int name##_size(const struct name *); \
   f bool name##_contains(const struct name *, ktype)

#define _set_gen_body(name, ktype, ktype_ref, ktype_tn, ktype_in, ktype_out, f)\
   _adt_func_body(name, f);                                                    \
   _gen_iter_body(name, ktype_ref, void *, f);                                 \
                                                                               \
   static uint64_t name##_hash(const void *a)                                  \
   {                                                                           \
      return ktype_tn##_hash((const ktype_ref)a);                              \
   }                                                                           \
   static int name##_key_compare(const void *a, const void *b)                 \
   {                                                                           \
      return ktype_tn##_compare((const ktype_ref)a, (const ktype_ref)b);       \
   }                                                                           \
   f void name##_init(struct name *a)                                          \
   {                                                                           \
      map_init(&a->map);                                                       \
      a->map.hash = name##_hash;                                               \
      a->map.key_compare = name##_key_compare;                                 \
   }                                                                           \
   f void name##_destroy(struct name *a) {                                     \
      ktype_ref k; void *v;                                                    \
      struct aiter i; i.ipos = 0;                                              \
      while (map_iterate(&a->map, &i, (void **)&k, &v)) {                      \
         ktype_tn##_free(k);                                                   \
      }                                                                        \
      map_destroy(&a->map);                                                    \
   }                                                                           \
   f bool name##_iter_next(const struct name *a, struct name##_iter *i)        \
   {                                                                           \
      void *v = NULL;                                                          \
      if (i->pos.ipos == 0)                                                    \
         i->key--;                                                             \
      i->key++;                                                                \
      return map_iterate(&a->map, &i->pos, (void **)&i->value,                 \
         (void **)&v);                                                         \
   }                                                                           \
   static void name##_insert_(struct name *a, ktype k)                         \
   {                                                                           \
      ktype_ref nk = ktype_tn##_new();                                         \
      ktype_tn##_copy(nk, ktype_in(k));                                        \
      void *to = NULL;                                                         \
      ktype_ref ko = NULL;                                                     \
      map_insert(&a->map, nk, NULL, (void **)&ko, (void **)&to);               \
      if (ko != NULL)                                                          \
         ktype_tn##_free(ko);                                                  \
   }                                                                           \
   f void name##_insert(struct name *a, ktype k)                               \
   {                                                                           \
      name##_insert_(a, k);                                                    \
   }                                                                           \
   f bool name##_remove(struct name *a, ktype k)                               \
   {                                                                           \
      ktype_ref ko;                                                            \
      void *to = NULL;                                                         \
      if (!map_remove(&a->map, ktype_in(k), (void **)&ko, (void **)&to))       \
         return false;                                                         \
      ktype_tn##_free(ko);                                                     \
      return true;                                                             \
   }                                                                           \
   f int name##_size(const struct name *a)                                     \
   {                                                                           \
      return map_size(&a->map);                                                \
   }                                                                           \
   f bool name##_contains(const struct name *a, ktype k)                       \
   {                                                                           \
      return map_contains(&a->map, ktype_in(k));                               \
   }                                                                           \
   f void name##_copy(struct name *dst, const struct name *src)                \
   {                                                                           \
      name##_destroy(dst); name##_init(dst);                                   \
      ktype_ref k;                                                             \
      void *v = NULL;                                                          \
      struct aiter i; i.ipos = 0;                                              \
      while (map_iterate(&src->map, &i, (void **)&k, (void **)&v))             \
         name##_insert_(dst, ktype_out(k));                                    \
   }                                                                           \
   SWALLOWSEMICOLON

#define ref(x) &x
#define unref(x) *x

// POD
#define set_gen_pod_header(name, ktype) \
   _set_gen_header(name, ktype, ktype *, )
#define set_gen_pod_body(name, ktype) \
   _set_gen_body(name, ktype, ktype *, ktype, ref, unref, )

#define set_gen_pod_static(name, ktype) \
   _set_gen_header(name, ktype, ktype *, static); \
   _set_gen_body(name, ktype, ktype *, ktype, ref, unref, static)

// type
#define set_gen_header(name, ktype) \
   _set_gen_header(name, const struct ktype *, struct ktype *, )
#define set_gen_body(name, ktype) \
   _set_gen_body(name, const struct ktype *, struct ktype *, ktype, , , )

#define set_gen_static(name, ktype) \
   _set_gen_header(name, const struct ktype *, struct ktype *, static); \
   _set_gen_body(name, const struct ktype *, struct ktype *, ktype, , , static)

