#ifndef __STRING_H
#define __STRING_H
#include <adt.h>
#include <stdtyp/vector.h>

#define create_string(name, str) \
   struct string name a_cleanup(string_destroy) = \
   string_make_var(str)

#define create_ptr_string(name, str) \
   struct string *name a_cleamup(string_freer) = \
   string_new_var(str)

#define strw(cstr) \
   &((const struct string){ .buff = (char *)cstr, \
   .buff_len = sizeof(cstr) / sizeof(char), \
   .length = sizeof(cstr) / sizeof(char) - 1, \
   .mallocd = false })

void
carr_freer(const char ***);

#define create_cstr_array(name, expr) \
   struct string_vec *____sv = expr; \
   const char **name a_cleanup(carr_freer) = \
      (const char **)malloc(sizeof(char *) * (string_vec_size(____sv) + 1)); \
   for (int i = 0; i < string_vec_size(____sv); i++) { \
      name[i] = string_to_cstring(string_vec_at(____sv, i)); } \
   name[string_vec_size(____sv)] = NULL;

struct string {
   char *buff;
   uint64_t buff_len;
   uint64_t length;
   bool mallocd;
};
adt_func_header(string);

vector_gen_header(string_vec, string);

#define create_string_vec(name, ...) \
   struct string_vec name a_cleanup(string_vec_destroy) = \
      _strvw(strw(#__VA_ARGS__))

struct string_vec
_strvw(const struct string *);

struct string
string_make_const(const char *buff);

const struct string *
string_new_const(const char *buff);

void
string_append_cstring(struct string *, const char *);

void
string_append_cstring_size(struct string *, const char *, size_t);

void
string_append_string(struct string *, const struct string *);

struct error
string_read_fd(struct string *, int);

struct string
string_make_var(const char *cstr);

struct string *
string_new_var(const char *cstr);

const char *
string_to_cstring(const struct string *);

void
string_clear(struct string *);

void
string_append_char(struct string *s, char);

void
string_append_int(struct string *, int);

void
string_set_cstring(struct string *s, char *);

uint64_t
string_length(const struct string *);

char
string_char_at_index(const struct string *, uint64_t);

struct string *
string_substring(struct string *, uint64_t, int len);

int
string_to_int(const struct string *);

bool
string_equal(const struct string *, const struct string *);

struct error
string_write_to_file(const struct string *, const struct string *);

struct error
string_read_from_file(struct string *, const struct string *);

void
string_split(const struct string *, char c, struct string_vec *);

void
string_tokenize(const struct string *, struct string_vec *);

bool
string_ends_with(const struct string *, const struct string *);

bool
string_contains_char(const struct string *, char);

bool
string_contains_substring(const struct string *, const struct string *);

#endif // __STRING_H
