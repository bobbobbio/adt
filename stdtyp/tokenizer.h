// Copyright 2015 Remi Bernotavicius. All right reserved.

#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

#include <adt.h>
#include <stdtyp/string.h>

struct tokenizer {
   struct stream *stream;
   struct string buff;
   uint64_t pos;
   struct string skip_chars;
   bool allow_empty;
};
adt_func_header(tokenizer);

#define create_tokenizer(name, stream) \
   struct tokenizer name a_cleanup(tokenizer_destroy) = \
   tokenizer_make_var(stream)

#define create_ptr_tokenizer(name, stream) \
   struct tokenizer *name a_cleanup(tokenizer_freer) = \
   tokenizer_new_var(stream)

struct tokenizer
tokenizer_make_var(struct stream *);

struct tokenizer *
tokenizer_new_var(struct stream *);

void
tokenizer_set_stream(struct tokenizer *, struct stream *);

bool
tokenizer_get_next(struct tokenizer *, struct string *);

void
tokenizer_set_skip_chars(struct tokenizer *, struct string *);

void
tokenizer_set_allow_empty(struct tokenizer *, bool);

#endif // __TOKENIZER_H__
