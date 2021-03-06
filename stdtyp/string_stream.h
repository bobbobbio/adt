// Copyright 2015 Remi Bernotavicius. All right reserved.

#ifndef __STRING_STREAM_H__
#define __STRING_STREAM_H__

#include <adt.h>
#include <stdtyp/stream.h>

struct a_packed string_stream {
   enum stream_type type;
   struct string *string;
   unsigned offset;
   bool is_const;
};
adt_func_header(string_stream);

struct stream *
string_stream_to_stream(struct string_stream *);

#define create_string_stream(name, string) \
   struct string_stream name a_cleanup(string_stream_destroy) = \
      string_stream_make_var(string)

#define create_string_stream_const(name, string) \
   struct string_stream name a_cleanup(string_stream_destroy) = \
      string_stream_const_make_var(string)

struct string_stream
string_stream_make_var(struct string *);

struct string_stream
string_stream_const_make_var(const struct string *);

struct error
string_stream_read(struct stream *, struct string *, size_t want, size_t *got)
   a_warn_unused_result;

struct error
string_stream_write(struct stream *, const struct string *)
   a_warn_unused_result;

bool
string_stream_has_more(struct stream *);

#endif // __STRING_STREAM_H__


