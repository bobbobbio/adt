#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

#include <adt.h>
#include <stdtyp/string.h>

struct tokenizer {
   struct string *data;
   uint64_t pos;
   struct string skip_chars;
   bool allow_empty;
};
adt_func_header(tokenizer);

#define create_tokenizer(name, str) \
   struct tokenizer name a_cleanup(tokenizer_destroy) = \
   tokenizer_make_var(str)

#define create_ptr_tokenizer(name, str) \
   struct tokenizer *name a_cleanup(tokenizer_freer) = \
   tokenizer_new_var(str)

struct tokenizer
tokenizer_make_var(const struct string *);

struct tokenizer *
tokenizer_new_var(const struct string *);

void
tokenizer_set_string(struct tokenizer *, const struct string *);

bool
tokenizer_get_next(struct tokenizer *, struct string *);

bool
tokenizer_has_next(struct tokenizer *);

void
tokenizer_set_skip_chars(struct tokenizer *, const struct string *);

void
tokenizer_set_allow_empty(struct tokenizer *, bool);

#endif // __TOKENIZER_H__
