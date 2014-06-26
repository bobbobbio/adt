#ifndef __ADT_H
#define __ADT_H

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// XXX These don't go here
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

// Use a random extern symbol to swallow a semicolon
#define SWALLOWSEMICOLON extern void __BODYCODEGENEND99893__

#define a_cleanup(n) __attribute__ ((__cleanup__(n)))

#define create(type, name) \
   struct type name a_cleanup(type##_destroy) = type##_make()

#define create_copy(type, name, copy) \
   struct type name a_cleanup(type##_destroy) = type##_make(); \
   type##_copy(&name, copy)

#define type_malloc(type) \
   ((struct type *)malloc(sizeof (struct type)))

#define create_ptr(type, name) \
   struct type *name a_cleanup(type##_freer) = \
   type##_new()

#define create_null_ptr(type, name) \
   struct type *name a_cleanup(type##_freer) = NULL

#define _adt_func_header(type, f) \
   f struct type * type##_new(void); \
   f void type##_free(struct type *); \
   f void type##_freer(struct type **); \
   f void type##_init(struct type *); \
   f struct type type##_make(void); \
   f void type##_destroy(struct type *); \
   f void type##_print(const struct type *); \
   f void type##_copy(struct type *, const struct type *)

#define _adt_func_body(type, f) \
   f struct type type##_make(void) { \
   struct type a; type##_init(&a); return a; } \
   f struct type * type##_new(void) { \
   struct type *a = type_malloc(type); \
   type##_init(a); return a; } \
   f void type##_free(struct type *a) { \
   type##_destroy(a); \
   free(a); } \
   f void type##_freer(struct type **a) { \
   if (*a != NULL) \
      type##_free(*a); } \
   SWALLOWSEMICOLON

#define adt_func_body(type) _adt_func_body(type, )
#define adt_func_header(type) _adt_func_header(type, )
#define adt_func_static(type) \
   _adt_func_header(type, static); \
   _adt_func_body(type, static)

// The POD func gen is the same as the regular except it gives you an init that
// zeros out the struct and a do nothing destructor, and implements copy, equal,
// compare and hash
#define _adt_func_pod_body(type, f) \
   _adt_func_body(type, f); \
   f void type##_init(struct type *a) { memset(a, 0, sizeof(struct type)); } \
   f void type##_destroy(struct type *a) {} \
   f void type##_copy(struct type *a, const struct type *b) { *a = *b; } \
   f bool type##_equal(const struct type *a, const struct type *b) \
      { return memcmp((const void *)a, (const void *)b, \
      sizeof(struct type)) == 0; } \
   f int type##_compare(const struct type *a, const struct type *b) \
      { return memcmp((const void *)a, (const void *)b, \
      sizeof(struct type)); } \
   f uint64_t type##_hash(const struct type *a) \
      { return memory_hash((void *)a, sizeof(struct type)); } \
   SWALLOWSEMICOLON
#define adt_func_pod_body(type) _adt_func_pod_body(type, )
#define adt_func_pod_header(type) _adt_func_header(type, )
#define adt_func_pod_static(type) \
   _adt_func_header(type, static); \
   _adt_func_pod_body(type, static)

static uint64_t
memory_hash(void *m, size_t size)
{
   uint64_t hash = 0;

   uint8_t *h8 = (uint8_t *)&hash;
   uint8_t *m8 = m;

   for (int i = 0; i < size; i++) {
      h8[i % 8] ^= m8[i];
   }

   return hash;
}

// POD types, printing
#define int_print(x) printf("%d", *(x))
#define uint64_t_print(x) printf("%" SCNu64, *(x))
#define uint32_t_print(x) printf("%" SCNu32, *(x))
#define uint16_t_print(x) printf("%" SCNu16, *(x))
#define uint8_t_print(x) printf("%" SCNu8, *(x))
#define int64_t_print(x) printf("%" SCNd64, *(x))
#define int32_t_print(x) printf("%" SCNd32, *(x))
#define int16_t_print(x) printf("%" SCNd16, *(x))
#define int8_t_print(x) printf("%" SCNd8, *(x))
#define float_print(x) printf("%f", *(x))
#define double_print float_print
#define char_print(x) printf("%c", *(x))
#define unsigned_print(x) printf("%u", *(x))
#define void_print(x) printf("%p", *(x))

// POD types malloc and free
#define m_make(name, type) \
   static type *name##_new() { \
      return malloc(sizeof(type)); } \
   static void name##_init(type *p) {} \
   static void name##_destroy(type *p) {} \
   static bool name##_equal(type *a, type *b) { return *a == *b; } \
   static void name##_free(type *p) { \
      free(p); } \
   static void name##_copy(type *d, type *s) { \
      *d = *s; } \
   SWALLOWSEMICOLON

#define pod_m_make(type) \
   m_make(type, type); \
   static uint64_t type##_hash(const type *a) { \
      return *a; } \
   static int type##_compare(const type *a, const type *b) { \
      return *a - *b; } \
   SWALLOWSEMICOLON

pod_m_make(int);
pod_m_make(unsigned);
pod_m_make(char);
pod_m_make(double);
pod_m_make(float);
pod_m_make(uint64_t);
pod_m_make(uint32_t);
pod_m_make(uint16_t);
pod_m_make(uint8_t);
pod_m_make(int64_t);
pod_m_make(int32_t);
pod_m_make(int16_t);
pod_m_make(int8_t);

// Convert existing structs to use create, that have existing free
#define _convert_ctype_body(type, f_func, f) \
   f void \
   type##_init(struct type *a) { memset(a, 0, sizeof(struct type)); } \
   f struct type \
   type##_make() { struct type a; type##_init(&a); return a; } \
   f void \
   type##_destroy(struct type *a) { } \
   f void \
   type##_freer(struct type **a) { if (*a != NULL) f_func(*a); } \
   SWALLOWSEMICOLON

#define _convert_ctype_header(type, f_func, f) \
   f void type##_init(struct type *); \
   f struct type type##_make(); \
   f void type##_destroy(struct type *a); \
   f void type##_freer(struct type **a); \
   SWALLOWSEMICOLON

#define convert_ctype_header(type, f_func) \
   _convert_ctype_header(type, f_func, )

#define convert_ctype_body(type, f_func) \
   _convert_ctype_body(type, f_func, )

#define convert_ctype_static(type, f_func) \
   _convert_ctype_header(type, f_func, static); \
   _convert_ctype_body(type, f_func, static)

#include <error.h>

#endif // __ADT_H
