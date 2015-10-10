// Copyright 2015 Remi Bernotavicius. All right reserved.

#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <adt.h>
#include <stdtyp/iter.h>

struct vector {
   char *buff;
   uint64_t buff_len;
   uint64_t size;
};
adt_func_header(vector);

void *
vector_at(const struct vector *, uint64_t, size_t);

void *
vector_insert(struct vector *, uint64_t, size_t);

void *
vector_extend(struct vector *, int, size_t);

void
vector_remove(struct vector *, uint64_t, size_t);

uint64_t
vector_size(const struct vector *);

void
vector_init_size(struct vector *, uint64_t, size_t);

bool
vector_iterate(const struct vector *, struct aiter *, uint64_t *, void **,
   size_t);

void
vector_sort(struct vector *, int (*compare)(const void *, const void *),
   size_t);

#define _vector_gen_header(name, type, type_ref, f) \
   struct name { struct vector vector; }; \
   _adt_func_header(name, f); \
   _gen_iter_header(name, int, type_ref, f); \
   a_unused f struct name *name##_new_size(uint64_t); \
   a_unused f bool name##_equal(const struct name *, const struct name *); \
   a_unused f void name##_insert(struct name *, type, int i); \
   a_unused f void name##_append(struct name *, type); \
   a_unused f void name##_extend(struct name *, const struct name *); \
   a_unused f void name##_prepend(struct name *, type); \
   a_unused f type_ref name##_grow(struct name *); \
   a_unused f void name##_remove(struct name *, int i); \
   a_unused f void name##_remove_last(struct name *); \
   a_unused f int name##_index_of(struct name *, type); \
   a_unused f bool name##_remove_value(struct name *, type); \
   a_unused f void name##_get(const struct name *, type_ref, int); \
   a_unused f type_ref name##_at(const struct name *, int); \
   a_unused f void name##_init_size(struct name *, uint64_t); \
   a_unused f bool name##_contains(const struct name *, type); \
   a_unused f void name##_clear(struct name *); \
   a_unused f int name##_size(const struct name *); \
   a_unused f void name##_resize(struct name *, size_t); \
   a_unused f void name##_sort(struct name *, \
      int (*comparator)(const type_ref, const type_ref))

#define _vector_gen_body(name, type, type_ref, type_in, so, typename, f)       \
   _adt_func_body(name, f);                                                    \
   _gen_iter_body(name, int, type_ref, f);                                     \
   f bool name##_iter_next(const struct name *n, struct name##_iter *i)        \
   {                                                                           \
      uint64_t key;                                                            \
      if (!vector_iterate(&n->vector, &i->pos, &key, (void **)&i->value, so))  \
         return false;                                                         \
      i->key = key;                                                            \
      return true;                                                             \
   }                                                                           \
   f struct name *name##_new_size(uint64_t size)                               \
   {                                                                           \
      struct name *a = type_malloc(name);                                      \
      name##_init_size(a, size);                                               \
      for (unsigned i = 0; i < size; i++)                                      \
         typename##_init(vector_at(&a->vector, i, so));                        \
      return a;                                                                \
   }                                                                           \
   f bool name##_equal(const struct name *a, const struct name *b)             \
   {                                                                           \
      if (name##_size(a) != name##_size(b))                                    \
         return false;                                                         \
      for (unsigned i = 0; i < name##_size(a); i++) {                          \
         if (!typename##_equal(name##_at(a, i), name##_at(b, i)))              \
            return false;                                                      \
      }                                                                        \
      return true;                                                             \
   }                                                                           \
   f void name##_insert(struct name *a, type v, int i)                         \
   {                                                                           \
      type_ref w = vector_insert(&a->vector, i, so);                           \
      typename##_init(w);                                                      \
      typename##_copy(w, type_in (v));                                         \
   }                                                                           \
   f void name##_append(struct name *a, type v)                                \
   {                                                                           \
      type_ref w = vector_insert(&a->vector, vector_size(&a->vector), so);     \
      typename##_init(w);                                                      \
      typename##_copy(w, type_in (v));                                         \
   }                                                                           \
   f void name##_extend(struct name *a, const struct name *b)                  \
   {                                                                           \
      type_ref w = vector_extend(&a->vector, name##_size(b), so);              \
      for (unsigned i = 0; i < name##_size(b); i++) {                          \
         typename##_init(w);                                                   \
         typename##_copy(w, name##_at(b, i));                                  \
         w++;                                                                  \
      }                                                                        \
   }                                                                           \
   f type_ref name##_grow(struct name *a)                                      \
   {                                                                           \
      type_ref w = vector_insert(&a->vector, vector_size(&a->vector), so);     \
      typename##_init(w);                                                      \
      return w;                                                                \
    }                                                                          \
   f void name##_prepend(struct name *a, type v)                               \
   {                                                                           \
      type_ref w = vector_insert(&a->vector, 0, so);                           \
      typename##_init(w);                                                      \
      typename##_copy(w, type_in (v));                                         \
   }                                                                           \
   f void name##_remove(struct name *a, int i)                                 \
   {                                                                           \
      typename##_destroy(vector_at(&a->vector, i, so));                        \
      vector_remove(&a->vector, i, so);                                        \
   }                                                                           \
   f void name##_remove_last(struct name *a)                                   \
   {                                                                           \
      name##_remove(a, name##_size(a) - 1);                                    \
   }                                                                           \
   f void name##_get(const struct name *a, type_ref r, int i)                  \
   {                                                                           \
      typename##_copy(r, vector_at(&a->vector, i, so));                        \
   }                                                                           \
   f type_ref name##_at(const struct name *a, int i)                           \
   {                                                                           \
      return vector_at(&a->vector, i, so);                                     \
   }                                                                           \
   f void name##_init(struct name *n)                                          \
   {                                                                           \
      vector_init(&n->vector);                                                 \
   }                                                                           \
   f void name##_init_size(struct name *n, uint64_t size)                      \
   {                                                                           \
      vector_init_size(&n->vector, size, so);                                  \
   }                                                                           \
   f void name##_destroy(struct name *a)                                       \
   {                                                                           \
      name##_clear(a);                                                         \
      vector_destroy(&a->vector);                                              \
   }                                                                           \
   f int name##_size(const struct name *a)                                     \
   {                                                                           \
      return vector_size(&a->vector);                                          \
   }                                                                           \
   f void name##_sort(struct name *a,                                          \
      int (*comparator)(const type_ref, const type_ref))                       \
   {                                                                           \
      vector_sort(&a->vector,                                                  \
         (int (*)(const void *, const void *))comparator, so);                 \
   }                                                                           \
   f int name##_index_of(struct name *a, type v)                               \
   {                                                                           \
      for (unsigned i = 0; i < vector_size(&a->vector); i++) {                 \
         if (typename##_equal(vector_at(&a->vector, i, so), type_in(v)))       \
            return i;                                                          \
      }                                                                        \
      return -1;                                                               \
   }                                                                           \
   f bool name##_remove_value(struct name *a, type v)                          \
   {                                                                           \
      int i = name##_index_of(a, v);                                           \
      if (i == -1)                                                             \
         return false;                                                         \
      name##_remove(a, i);                                                     \
      return true;                                                             \
   }                                                                           \
   f void name##_print(const struct name *a, struct string *s)                 \
   {                                                                           \
      string_append_cstring(s, "[ ");                                          \
      if (vector_size(&a->vector) > 0)                                         \
         typename##_print((const type_ref)vector_at(&a->vector, 0, so), s);    \
      for (unsigned i = 1; i < vector_size(&a->vector); i++) {                 \
         string_append_cstring(s, ", ");                                       \
         typename##_print((const type_ref)vector_at(&a->vector, i, so), s);    \
      }                                                                        \
      string_append_cstring(s, " ]");                                          \
   }                                                                           \
   f bool name##_contains(const struct name *a, type v)                        \
   {                                                                           \
      for (unsigned i = 0; i < name##_size(a); i++) {                          \
         if (typename##_equal(type_in (v), name##_at(a, i)))                   \
            return true;                                                       \
      }                                                                        \
      return false;                                                            \
   }                                                                           \
   f void name##_clear(struct name *a)                                         \
   {                                                                           \
      while (vector_size(&a->vector) > 0)                                      \
         name##_remove(a, name##_size(a) - 1);                                 \
   }                                                                           \
   f void name##_resize(struct name *a, size_t new_size)                       \
   {                                                                           \
      if (new_size > vector_size(&a->vector)) {                                \
         unsigned expand_by = new_size - vector_size(&a->vector);              \
         type_ref w = vector_extend(&a->vector, expand_by, so);                \
         for (unsigned i = 0; i < expand_by; i++) {                            \
            typename##_init(w);                                                \
            w++;                                                               \
         }                                                                     \
      } else {                                                                 \
         while (vector_size(&a->vector) > new_size)                            \
            name##_remove(a, name##_size(a) - 1);                              \
      }                                                                        \
   }                                                                           \
   f void name##_copy(struct name *d, const struct name *s)                    \
   {                                                                           \
      name##_clear(d);                                                         \
      for (unsigned i = 0; i < vector_size(&s->vector); i++) {                 \
         type_ref w = vector_insert(&d->vector, vector_size(&d->vector), so);  \
         typename##_init(w);                                                   \
         typename##_copy(w, vector_at(&s->vector, i, so));                     \
      }                                                                        \
   }                                                                           \
   SWALLOWSEMICOLON

#define ref(x) &x

#define __vector_gen_body(name, type, f) \
   _vector_gen_body(name, const struct type *, struct type *, \
    , sizeof(struct type), type, f)
#define __vector_gen_header(name, type, f) \
   _vector_gen_header(name, const struct type *, struct type *, f)

#define __vector_gen_pod_body(name, type, f) \
   a_unused static void name##_type_print(const type *a, struct string *s) \
   { type##_print(a, s); } \
   _vector_gen_body(name, type, type *, ref, sizeof(type), type, f)
#define __vector_gen_pod_header(name, type, f) \
   _vector_gen_header(name, type, type *, f)

#define vector_gen_header(name, type) \
   __vector_gen_header(name, type, )
#define vector_gen_body(name, type) \
   __vector_gen_body(name, type, )

#define vector_gen_pod_header(name, type) \
   __vector_gen_pod_header(name, type, )
#define vector_gen_pod_body(name, type) \
   __vector_gen_pod_body(name, type, )

#define vector_gen_static(name, type) \
   __vector_gen_header(name, type, static); \
   __vector_gen_body(name, type, static)

#define vector_gen_pod_static(name, type) \
   __vector_gen_pod_header(name, type, static); \
   __vector_gen_pod_body(name, type, static)

// Ptr Vec
#define _vector_gen_ptr_header(name, type, f) \
   _vector_gen_header(name, struct type *, struct type **, f); \
   a_unused f void name##_free_ptrs(struct name *)

#define _vector_gen_ptr_body(name, type, f) \
   m_make(name##_void, struct type *); \
   static void name##_void_print(const struct type **a, struct string *s) \
   { void_print(a, s); } \
   f void name##_free_ptrs(struct name *a) { \
      for (unsigned i = 0; i < name##_size(a); i++) \
      type##_free(*name##_at(a, i)); } \
   _vector_gen_body(name, struct type *, struct type **, ref, \
      sizeof(struct type *), name##_void, f)

#define vector_gen_ptr_header(name, type) \
   _vector_gen_ptr_header(name, type, )

#define vector_gen_ptr_body(name, type) \
   _vector_gen_ptr_body(name, type, )

#define vector_gen_ptr_static(name, type) \
   _vector_gen_ptr_header(name, type, static); \
   _vector_gen_ptr_body(name, type, static)

#include <stdtyp/string.h>

vector_gen_pod_header(int_vec, int);

#define create_int_vec(name, ...) \
   struct int_vec name a_cleanup(int_vec_destroy) = \
      int_vec_make_var((int[]){__VA_ARGS__}, \
         sizeof((int[]){__VA_ARGS__}) / sizeof(int))

struct int_vec
int_vec_make_var(int val[], int len);

#endif // __VECTOR_H__
