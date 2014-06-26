#include <adt.h>
#include <stdtyp/vector.h>
#include <stdtyp/iter.h>

struct map_table;

struct map {
   struct map_table *buckets;
   uint64_t size;

   int (*key_compare)(const void *, const void *);
   uint64_t (*hash)(const void *);
};
adt_func_header(map);

void **
map_at(const struct map *, const void *);

void *
map_get(const struct map *, const void *);

void
map_insert(struct map *, void *, void *, void **, void **);

bool
map_remove(struct map *, void *, void **, void **);

uint64_t
map_size(const struct map *);

void
map_clear(struct map *);

bool
map_contains(const struct map *, const void *);

bool
map_iterate(const struct map *, struct aiter *, void **, void **);

// hashers for basic types
// XXX the compares should really not go here, also as written they are only
// good for equality
uint64_t uint64_t_hash(const void *_key);
int uint64_t_compare(const void *a, const void *b);
uint64_t uint32_t_hash(const void *_key);
int uint32_t_compare(const void *a, const void *b);
uint64_t uint16_t_hash(const void *_key);
int uint16_t_compare(const void *a, const void *b);
uint64_t uint8_t_hash(const void *_key);
int uint8_t_compare(const void *a, const void *b);
uint64_t int_hash(const void *_key);
int int_compare(const void *a, const void *b);
#define int64_t_hash uint64_t_hash
#define int64_t_compare uint64_t_compare
#define int32_t_hash uint32_t_hash
#define int32_t_compare uint32_t_compare
#define int16_t_hash uint16_t_hash
#define int16_t_compare uint16_t_compare
#define int8_t_hash uint8_t_hash
#define int8_t_compare uint8_t_compare
#define float_hash int_hash
#define float_compare int_compare
#define double_hash uint32_t_hash
#define double_compare uint32_t_compare
#define char_hash uint8_t_hash
#define char_compare uint8_t_compare
#define unsigned_hash int_hash
#define unsigned_compare int_compare

uint64_t string_hash(const void *_key);
int string_compare(const void *a, const void *b);

#define _map_gen_header(name, ktype, ktype_ref, type, type_ref, cnst, f) \
   struct name { struct map map; }; \
   _adt_func_header(name, f); \
   _gen_iter_header(name, ktype_ref, type_ref, f); \
   f bool name##_iter_next(const struct name *, struct name##_iter *); \
   f void name##_get(const struct name *, ktype, type_ref); \
   f type_ref name##_at(const struct name *, ktype); \
   f void name##_insert(struct name *, ktype, cnst type); \
   f bool name##_remove(struct name *, ktype); \
   f int name##_size(const struct name *); \
   f void name##_copy(struct name *, const struct name *); \
   f bool name##_contains(struct name *, ktype)

#define _map_gen_body(name, ktype, ktype_ref, ktype_tn, ktype_in, ktype_out,   \
      typename, type, type_ref, vref, cnst, f)                                 \
   _adt_func_body(name, f);                                                    \
   _gen_iter_body(name, ktype_ref, type_ref, f);                               \
                                                                               \
   f void name##_init(struct name *a)                                          \
   {                                                                           \
      map_init(&a->map);                                                       \
      a->map.hash = ktype_tn##_hash;                                           \
      a->map.key_compare = ktype_tn##_compare;                                 \
   }                                                                           \
   f void name##_destroy(struct name *a) {                                     \
      ktype_ref k; type_ref v;                                                 \
      struct aiter i; i.ipos = 0;                                              \
      while (map_iterate(&a->map, &i, (void **)&k, (void **)&v)) {             \
         typename##_free(v);                                                   \
         ktype_tn##_free(k);                                                   \
      }                                                                        \
      map_destroy(&a->map);                                                    \
   }                                                                           \
   f bool name##_iter_next(const struct name *a, struct name##_iter *i)        \
   {                                                                           \
      return map_iterate(&a->map, &i->pos, (void **)&i->key,                   \
         (void **)&i->value);                                                  \
   }                                                                           \
   f void name##_get(const struct name *a, ktype k, type_ref o)                \
   {                                                                           \
      type_ref v = map_get(&a->map, ktype_in(k));                              \
      assert(v != NULL);                                                       \
      typename##_copy(o, v);                                                   \
   }                                                                           \
   f type_ref name##_at(const struct name *a, ktype k)                         \
   {                                                                           \
      return (type_ref)map_get(&a->map, ktype_in(k));                          \
   }                                                                           \
   static void name##_insert_(struct name *a, ktype k, cnst type_ref v)        \
   {                                                                           \
      type_ref nv = typename##_new();                                          \
      typename##_copy(nv, v);                                                  \
      ktype_ref nk = ktype_tn##_new();                                         \
      ktype_tn##_copy(nk, ktype_in(k));                                        \
      type_ref to = NULL;                                                      \
      ktype_ref ko = NULL;                                                     \
      map_insert(&a->map, nk, nv, (void **)&ko, (void **)&to);                 \
      if (to != NULL)                                                          \
         typename##_free(to);                                                  \
      if (ko != NULL)                                                          \
         ktype_tn##_free(ko);                                                  \
   }                                                                           \
   f void name##_insert(struct name *a, ktype k, cnst type v)                  \
   {                                                                           \
      name##_insert_(a, k, vref(v));                                           \
   }                                                                           \
   f bool name##_remove(struct name *a, ktype k)                               \
   {                                                                           \
      ktype_ref ko;                                                            \
      type_ref to;                                                             \
      if (!map_remove(&a->map, (void *)&k, (void **)&ko, (void **)&to))        \
         return false;                                                         \
      typename##_free(to);                                                     \
      ktype_tn##_free(ko);                                                     \
      return true;                                                             \
   }                                                                           \
   f int name##_size(const struct name *a)                                     \
   {                                                                           \
      return map_size(&a->map);                                                \
   }                                                                           \
   f bool name##_contains(struct name *a, ktype k)                             \
   {                                                                           \
      return map_contains(&a->map, ktype_in(k));                               \
   }                                                                           \
   f void name##_copy(struct name *dst, const struct name *src)                \
   {                                                                           \
      name##_destroy(dst); name##_init(dst);                                   \
      ktype_ref k;                                                             \
      type_ref v;                                                              \
      struct aiter i; i.ipos = 0;                                              \
      while (map_iterate(&src->map, &i, (void **)&k, (void **)&v))             \
         name##_insert_(dst, ktype_out(k), v);                                 \
   }                                                                           \
   SWALLOWSEMICOLON

#define ref(x) &x
#define unref(x) *x

// POD key
#define map_gen_podk_header(name, ktype, type) \
   _map_gen_header(name, ktype, ktype *, struct type *, struct type *, const, )
#define map_gen_podk_body(name, ktype, type) \
   _map_gen_body(name, ktype, ktype *, ktype, ref, unref, type, struct type *, \
      struct type *, , const, )

#define map_gen_podk_static(name, ktype, type) \
   _map_gen_header(name, ktype, ktype *, struct type *, struct type *, const, \
      static); \
   _map_gen_body(name, ktype, ktype *, ktype, ref, unref, type, struct type *, \
       struct type *, , const, static)

// type key
#define map_gen_header(name, ktype, type) \
   _map_gen_header(name, const struct ktype *, struct ktype *, struct type *, \
      struct type *, const, )
#define map_gen_body(name, ktype, type) \
   _map_gen_body(name, const struct ktype *, struct ktype *, ktype, , , \
      type, struct type *, struct type *, , const, )

#define map_gen_static(name, ktype, type) \
   _map_gen_header(name, const struct ktype *, struct ktype *, struct type *, \
      struct type *, const, static); \
   _map_gen_body(name, const struct ktype *, struct ktype *, ktype, , , type, \
      struct type *, struct type *, , const, static)

// ptr_value
// POD key
#define map_gen_podk_ptr_header(name, ktype, type) \
   _map_gen_header(name, ktype, ktype *, struct type *, struct type **, , )
#define map_gen_podk_ptr_body(name, ktype, type) \
   m_make(name##_void, struct type *); \
   static void name##_void_print(const struct type **a) { void_print(a); } \
   _map_gen_body(name, ktype, ktype *, ktype, ref, unref, name##_void, \
      struct type *, struct type **, ref, , )

#define map_gen_podk_ptr_static(name, ktype, type) \
   _map_gen_header(name, ktype, ktype *, struct type *, struct type **, \
      , static); \
   m_make(name##_void, struct type *); \
   static void name##_void_print(const struct type **a) { void_print(a); } \
   _map_gen_body(name, ktype, ktype *, ktype, ref, unref, name##_void, \
      struct type *, struct type **, ref, , static)

// type key
#define map_gen_ptr_header(name, ktype, type) \
   _map_gen_header(name, const struct ktype *, struct ktype *, struct type *, \
      struct type **, , )
#define map_gen_ptr_body(name, ktype, type) \
   m_make(name##_void, struct type *); \
   static void name##_void_print(const struct type **a) { void_print(a); } \
   _map_gen_body(name, const struct ktype *, struct ktype *, ktype, , , \
      name##_void, struct type *, struct type **, ref, , )

#define map_gen_ptr_static(name, ktype, type) \
   _map_gen_header(name, const struct ktype *, struct ktype *, struct type *, \
      struct type **, , static); \
   _map_gen_body(name, const struct ktype *, struct ktype *, ktype, , , void, \
      struct type *, struct type **, ref, , static)

// POD value
// POD key
#define map_gen_podk_podv_header(name, ktype, type) \
   _map_gen_header(name, ktype, ktype *, type, type *, , )
#define map_gen_podk_podv_body(name, ktype, type) \
   _map_gen_body(name, ktype, ktype *, ktype, ref, unref, type, \
      type, type *, ref, , )

#define map_gen_podk_podv_static(name, ktype, type) \
   _map_gen_header(name, ktype, ktype *, type, type *, , static); \
   _map_gen_body(name, ktype, ktype *, ktype, ref, unref, type, \
      type, type *, ref, , static)

// type key
#define map_gen_podv_header(name, ktype, type) \
   _map_gen_header(name, const struct ktype *, struct ktype *, type, \
      type *, , )
#define map_gen_podv_body(name, ktype, type) \
   _map_gen_body(name, const struct ktype *, struct ktype *, ktype, , , \
      type, type, type *, ref, , )

#define map_gen_podv_static(name, ktype, type) \
   _map_gen_header(name, const struct ktype *, struct ktype *, type, \
      type *, , static); \
   _map_gen_body(name, const struct ktype *, struct ktype *, ktype, , , \
      type, type, type *, ref, , static)
