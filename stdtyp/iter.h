#ifndef __ITER_H__
#define __ITER_H__

#include <adt.h>
#include <string.h>

// abstracted iterator position struct
struct aiter {
   uint64_t ipos;
   void *ppos;
};

#define _gen_iter_header(name, ktype, vtype, f) \
struct name##_iter { \
   ktype key; \
   vtype value; \
   struct aiter pos; \
}; \
_adt_func_header(name##_iter, f); \
f bool name##_iter_next(const struct name *, struct name##_iter *); \
typedef vtype name##_value; \
typedef ktype name##_key

#define _gen_iter_body(name, ktype, vtype, f) \
_adt_func_body(name##_iter, f); \
f void name##_iter_init(struct name##_iter *i) { \
   memset(i, 0, sizeof(struct name##_iter)); } \
f void name##_iter_destroy(struct name##_iter *i) {} \
SWALLOWSEMICOLON

#define iter_gen_header(name, ktype, vtype) \
   _gen_iter_header(name, ktype, vtype, )
#define iter_gen_body(name, ktype, vtype) \
   _gen_iter_body(name, ktype, vtype, )

#define iter_gen_static(name, ktype, vtype) \
   _gen_iter_header(name, ktype, vtype, static); \
   _gen_iter_body(name, ktype, vtype, static)

#define iter(type, sub, name) \
   for (struct type##_iter name = type##_iter_make(); \
      type##_iter_next(sub, &name); )

#define iter_value(type, sub, name) \
   for (struct type##_iter _iter = type##_iter_make(); \
      type##_iter_next(sub, &_iter); ) \
   for (bool _once = true; _once; ) \
   for (type##_value name = _iter.value; _once; _once = false)

#define iter_key(type, sub, name) \
   for (struct type##_iter _iter = type##_iter_make(); \
      type##_iter_next(sub, &_iter); ) \
   for (bool _once = true; _once; ) \
   for (type##_key name = _iter.key; _once; _once = false)

#endif // __ITER_H__
