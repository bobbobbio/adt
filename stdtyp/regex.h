#ifndef __REGEX_H__
#define __REGEX_H__

#include <adt.h>
#include <stdtyp/string.h>

#include <stdarg.h>
#include <regex.h>

#define create_regex(name, str) \
   struct regex name a_cleanup(regex_destroy) = regex_compile(str)

struct regex {
   regex_t r;
   uint64_t num_groups;
};

void
regex_destroy(struct regex *);

struct regex
regex_compile(const struct string *s);

#define regex_match(r, s, ...) \
   _regex_match(r, s, __VA_ARGS__, NULL)

bool
_regex_match(struct regex *, const struct string *s, ...);

bool
regex_match_all(struct regex *r, const struct string *s,
   struct string_vec_vec *matches_out);

#endif // __REGEX_H__
