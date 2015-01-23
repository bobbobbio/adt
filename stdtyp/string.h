#ifndef __STRING_H
#define __STRING_H
#include <adt.h>
#include <stdtyp/vector.h>

struct regex;

#define create_string(name, str) \
   struct string name a_cleanup(string_destroy) = \
   string_make_var(str)

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
vector_gen_header(string_vec_vec, string_vec);

char *
_printer(struct string_vec *sv, void(*p)(const void *, struct string *),
   const void *v);

// adt printing.  This section defines aprintf and its variants.  This function
// allows you to print adt types inside one function call. It turns this:
//
// create_string_vec(my_list, "a", "b", "c");
// {
//    create(string, temp);
//    string_vec_print(&my_list, &temp);
//    printf("%s", string_to_cstring(&temp))
// }
//
// into this:
//
// create_string_vec(my_list, "a", "b", "c");
// aprintf("%s", print(string_vec, &my_list));

#define print(type, v) \
   _printer(&__b__, (void(*)(const void *, struct string *))&type##_print, v)

#define adt_print(func, ...) \
do { \
   create(string_vec, __b__); \
   func(__VA_ARGS__); \
} while (0)

#define string_append_format(str, fmt, ...) \
   adt_print(_string_append_format, str, fmt, __VA_ARGS__)

#define aprintf(...) adt_print(printf, __VA_ARGS__)
#define afprintf(...) adt_print(fprintf, __VA_ARGS__)
#define asprintf(...) adt_print(sprintf, __VA_ARGS__)
#define asnprintf(...) adt_print(snprintf, __VA_ARGS__)
#define avprintf(...) adt_print(vprintf, __VA_ARGS__)
#define avfprintf(...) adt_print(vfprintf, __VA_ARGS__)
#define avsprintf(...) adt_print(vsprintf, __VA_ARGS__)
#define avsnprintf(...) adt_print(vsnprintf, __VA_ARGS__)

#define create_string_vec(name, ...) \
   struct string_vec name a_cleanup(string_vec_destroy) = \
      _strvw(__VA_ARGS__, NULL)

struct string_vec
_strvw(const char *a, ...);

struct string
string_const_make(const char *buff);

const struct string *
string_const_new(const char *buff);

void
string_append_cstring(struct string *, const char *);

void
string_append_cstring_length(struct string *, const char *, size_t);

void
string_append_string(struct string *, const struct string *);

// Read from given file descriptor.  Reads the whole file when length is 0.
// Don't call this, instead call file_read
struct error
string_read_fd(struct string *s, int fd, size_t length);

struct error
string_read_fd_non_blocking(struct string *s, int fd, size_t want, size_t *got);

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

int
string_to_int(const struct string *);

bool
string_equal(const struct string *, const struct string *);

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

void
string_remove_substring(struct string *, int s, int e);

int
string_compare(const struct string *, const struct string *);

uint64_t
string_hash(const struct string *s);

void
string_vec_join(struct string *, const struct string_vec *, char);

int
_string_append_format(struct string *, char *, ...);

bool
string_replace(struct string *s, const struct regex *r, const struct string *n);

bool
string_replace_first(struct string *s, const struct regex *r,
   const struct string *n);

#endif // __STRING_H
