// Copyright 2015 Remi Bernotavicius. All right reserved.

#ifndef __ADT_H
#define __ADT_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

struct string;
struct string_vec;

// XXX These don't go here
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

#define __unq(a, b, c) a##_##b##_##c
#define _unq(a, b, c) __unq(a, b, c)
#define unq(x) _unq(x, __COUNTER__, __LINE__)

#define noop(...)

#define once_loop(expr, once) \
   for (bool once = true; once; ) \
   for (expr; once; once = false)

#define ctx_def(expr) \
   once_loop(expr, unq(once))

// Use a random extern symbol to swallow a semicolon
#define SWALLOWSEMICOLON extern void __BODYCODEGENEND99893__

#define a_cleanup(n) __attribute__ ((__cleanup__(n)))
#define a_packed __attribute__ ((__packed__))
#define a_warn_unused_result __attribute__((warn_unused_result))
#define a_format(...) __attribute__((format(__VA_ARGS__)))
#define a_noreturn __attribute__((noreturn))
#define a_unused __attribute__((unused))
#define a_used __attribute__((used))

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

#define make(type, ...) \
   (struct type){ __VA_ARGS__ }

#define adt_printer_header(type) \
   /* We pass the print function in so that we don't link against it. */ \
   /* This avoids having to have the print function always defined. */ \
   char * type##_printer(const struct type *, struct string_vec *, \
      void(*p)(const struct type *, struct string *))

#define adt_printer_body(type) \
   char * type##_printer(const struct type *v, struct string_vec *sv, \
      void(*p)(const struct type *, struct string *)) \
   { \
      struct string *str = string_vec_grow(sv); \
      p(v, str); \
      return str->buff; \
   } \
   SWALLOWSEMICOLON

#define _adt_func_header(type, f) \
   a_unused f struct type * type##_new(void); \
   a_unused f void type##_free(struct type *); \
   a_unused f void type##_freer(struct type **); \
   a_unused f void type##_init(struct type *); \
   a_unused f struct type type##_make(void); \
   a_unused f void type##_destroy(struct type *); \
   a_unused f void type##_print(const struct type *, struct string *); \
   a_unused f void type##_copy(struct type *, const struct type *); \
   a_unused f adt_printer_header(type)

#define _adt_func_body(type, f) \
   f struct type type##_make(void) \
   { \
      struct type a; \
      type##_init(&a); \
      return a; \
   } \
   f struct type * type##_new(void) \
   { \
      struct type *a = type_malloc(type); \
      type##_init(a); \
      return a; \
   } \
   f void type##_free(struct type *a) \
   { \
      type##_destroy(a); \
      free(a); \
   } \
   f adt_printer_body(type); \
   f void type##_freer(struct type **a) \
   { \
      if (*a != NULL) \
         type##_free(*a); \
   } \
   SWALLOWSEMICOLON

// A little forward declaration is necessary here since adt uses these string
// and string vector functions.
struct string;
struct string_vec;
struct string *string_vec_grow(struct string_vec *);

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
   a_unused f void type##_init(struct type *a) { \
      memset(a, 0, sizeof(struct type)); } \
   a_unused f void type##_destroy(struct type *a) {} \
   a_unused f void type##_copy(struct type *a, const struct type *b) { \
      *a = *b; } \
   a_unused f bool type##_equal(const struct type *a, const struct type *b) \
      { return memcmp((const void *)a, (const void *)b, \
      sizeof(struct type)) == 0; } \
   a_unused f int type##_compare(const struct type *a, const struct type *b) \
      { return memcmp((const void *)a, (const void *)b, \
      sizeof(struct type)); } \
   a_unused f uint64_t type##_hash(const struct type *a) \
      { return memory_hash((void *)a, sizeof(struct type)); } \
   SWALLOWSEMICOLON
#define adt_func_pod_body(type) _adt_func_pod_body(type, )
#define adt_func_pod_header(type) \
   _adt_func_header(type, ); \
   int type##_compare(const struct type *a, const struct type *b); \
   uint64_t type##_hash(const struct type *a)
#define adt_func_pod_static(type) \
   _adt_func_header(type, static); \
   static int type##_compare(const struct type *a, const struct type *b); \
   static uint64_t type##_hash(const struct type *a); \
   _adt_func_pod_body(type, static)

a_unused
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
#define int_print(x, s) string_append_format(s, "%d", *(x))
#define uint64_t_print(x, s) string_append_format(s, "%" SCNu64, *(x))
#define uint32_t_print(x, s) string_append_format(s, "%" SCNu32, *(x))
#define uint16_t_print(x, s) string_append_format(s, "%" SCNu16, *(x))
#define uint8_t_print(x, s) string_append_format(s, "%" SCNu8, *(x))
#define int64_t_print(x, s) string_append_format(s, "%" SCNd64, *(x))
#define int32_t_print(x, s) string_append_format(s, "%" SCNd32, *(x))
#define int16_t_print(x, s) string_append_format(s, "%" SCNd16, *(x))
#define int8_t_print(x, s) string_append_format(s, "%" SCNd8, *(x))
#define float_print(x, s) string_append_format(s, "%f", *(x))
#define double_print float_print
#define char_print(x, s) string_append_format(s, "%c", *(x))
#define unsigned_print(x, s) string_append_format(s, "%u", *(x))
#define void_print(x, s) string_append_format(s, "%p", *(x))
#define pid_t_print(x, s) string_append_format(s, "%ld", (long)*(x))

// POD types malloc and free
#define m_make(name, type) \
   a_unused static type *name##_new() { \
      return malloc(sizeof(type)); } \
   a_unused static void name##_init(type *p) { memset(p, 0, sizeof(type)); } \
   a_unused static void name##_destroy(type *p) {} \
   a_unused static bool name##_equal(type *a, type *b) { return *a == *b; } \
   a_unused static void name##_free(type *p) { \
      free(p); } \
   a_unused static void name##_copy(type *d, type *s) { \
      *d = *s; } \
   SWALLOWSEMICOLON

#define pod_m_make(type) \
   m_make(type, type); \
   a_unused static uint64_t type##_hash(const type *a) { \
      return *a; } \
   a_unused static int type##_compare(const type *a, const type *b) { \
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
pod_m_make(pid_t);

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
#include <stdtyp/vector.h>

#endif // __ADT_H
