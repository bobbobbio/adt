// Copyright 2015 Remi Bernotavicius. All right reserved.

#ifndef __STRING_H
#define __STRING_H
#include <adt.h>
#include <stdtyp/vector.h>

struct regex;

#define create_string(name, str) \
   struct string name a_cleanup(string_destroy) = \
   string_make_var(str)

#define create_string_format(name, ...) \
   create(string, name); \
   string_append_format(&name, __VA_ARGS__)

#define create_ptr_string(name, str) \
   struct string *name a_cleamup(string_freer) = \
   string_new_var(str)

#define create_const_string(name, str) \
   struct string name = string_const_make(str)

#define strw(cstr) \
   &((const struct string){ .buff = (char *)cstr, \
   .buff_len = sizeof(cstr) / sizeof(char), \
   .length = sizeof(cstr) / sizeof(char) - 1, \
   .mallocd = false })

void
carr_freer(const char ***);

#define _create_cstr_array(name, expr, sv) \
   const struct string_vec *sv = expr; \
   const char **name a_cleanup(carr_freer) = \
      (const char **)malloc(sizeof(char *) * (string_vec_size(sv) + 1)); \
   for (int i = 0; i < string_vec_size(sv); i++) { \
      name[i] = string_to_cstring(string_vec_at(sv, i)); } \
   name[string_vec_size(sv)] = NULL

#define create_cstr_array(name, expr) \
   _create_cstr_array(name, expr, unq(sv))

struct string {
   char *buff;
   uint64_t buff_len;
   uint64_t length;
   bool mallocd;
};
adt_func_header(string);

vector_gen_header(string_vec, string);
vector_gen_header(string_vec_vec, string_vec);

// adt printing.  This section defines aprintf and its variants.  This function
// allows you to print adt types inside one function call. It turns this:
//
// create_string_vec_cstr(my_list, "a", "b", "c");
// {
//    create(string, temp);
//    string_vec_print(&my_list, &temp);
//    printf("%s", string_to_cstring(&temp))
// }
//
// into this:
//
// create_string_vec_cstr(my_list, "a", "b", "c");
// aprintf("%s", print(string_vec, &my_list));

#define print(type, v) \
   type##_printer(v, &__PRINT_BUFFER__, type##_print)

#define adt_print(func, ...) \
do { \
   create(string_vec, __PRINT_BUFFER__ a_unused); \
   func(__VA_ARGS__); \
} while (0)

#define string_append_format(str, fmt, ...) \
   adt_print(_string_append_format, str, fmt, ## __VA_ARGS__)

#define aprintf(...) adt_print(printf, __VA_ARGS__)
#define afprintf(...) adt_print(fprintf, __VA_ARGS__)
#define asprintf(...) adt_print(sprintf, __VA_ARGS__)
#define asnprintf(...) adt_print(snprintf, __VA_ARGS__)
#define avprintf(...) adt_print(vprintf, __VA_ARGS__)
#define avfprintf(...) adt_print(vfprintf, __VA_ARGS__)
#define avsprintf(...) adt_print(vsprintf, __VA_ARGS__)
#define avsnprintf(...) adt_print(vsnprintf, __VA_ARGS__)

#define create_string_vec_cstr(name, ...) \
   create(string_vec, name); \
   string_vec_append_cstrs(&name, __VA_ARGS__)

#define create_string_vec(name, ...) \
   create(string_vec, name); \
   string_vec_append_strings(&name, __VA_ARGS__)

#define string_vec_append_cstrs(name, ...) \
   _string_vec_append_cstrs(name, __VA_ARGS__, NULL)

#define string_vec_append_strings(name, ...) \
   _string_vec_append_strings(name, __VA_ARGS__, NULL)

void
_string_vec_append_cstrs(struct string_vec *self, const char *a, ...);

void
_string_vec_append_strings(
   struct string_vec *self, const struct string *a, ...);

struct string
string_const_make(const char *buff);

const struct string *
string_const_new(const char *buff);

void
string_strip(struct string *);

void
string_append_cstring(struct string *, const char *);

void
string_append_cstring_length(struct string *, const char *, size_t);

void
string_append_string(struct string *, const struct string *);

int
string_append_format_va_list(struct string *, const char *, va_list args);

struct error
string_read_fd(struct string *s, int fd, size_t want, size_t *got, bool *done)
   a_warn_unused_result;

struct string
string_make_var(const char *cstr);

struct string *
string_new_var(const char *cstr);

void
string_clear(struct string *);

void
string_append_char(struct string *s, char);

void
string_append_int(struct string *, int);

void
string_set_cstring(struct string *s, char *);

int
string_to_int(const struct string *);

bool
string_equal(const struct string *, const struct string *);

void
string_split(const struct string *, char c, struct string_vec *);

void
string_tokenize(const struct string *, struct string_vec *);

bool
string_starts_with(const struct string *, const struct string *);

bool
string_ends_with(const struct string *, const struct string *);

bool
string_contains_char(const struct string *, char);

bool
string_contains_substring(const struct string *, const struct string *);

void
string_remove_substring(struct string *, int s, int e);

int
string_compare(const struct string *, const struct string *);

uint64_t
string_hash(const struct string *s);

void
string_vec_join(struct string *, const struct string_vec *, char);

int
_string_append_format(struct string *, const char *, ...)
   a_format(printf, 2, 3);

bool
string_replace(struct string *s, const struct regex *r, const struct string *n);

bool
string_replace_first(struct string *s, const struct regex *r,
   const struct string *n);

static inline uint64_t
string_length(const struct string *s)
{
   return s->length;
}

static inline const char *
string_to_cstring(const struct string *s)
{
   return (const char *)s->buff;
}

inline static char
string_char_at_index(const struct string *s, uint64_t i)
{
   return s->buff[i];
}

static inline bool
char_is_whitespace(char c)
{
   return c == ' ' || c == '\t' || c  == '\n' || c == '\r';
}

#endif // __STRING_H
