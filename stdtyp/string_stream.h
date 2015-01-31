#ifndef __STRING_STREAM_H__
#define __STRING_STREAM_H__

#include <adt.h>
#include <stdtyp/stream.h>

struct a_packed string_stream {
   enum stream_type type;
   struct string *string;
   unsigned offset;
};
adt_func_header(string_stream);

#define create_string_stream(name, string) \
   struct string_stream name a_cleanup(string_stream_destroy) = \
      string_stream_make_var(string)

void
string_stream_set_string(struct string_stream *, struct string *);

struct string_stream
string_stream_make_var(struct string *);

struct error
string_stream_read(struct stream *, struct string *, size_t want, size_t *got)
   a_warn_unused_result;

struct error
string_stream_write(struct stream *, const struct string *)
   a_warn_unused_result;

bool
string_stream_has_more(struct stream *);

#define string_stream_interface { \
   .stream_read = string_stream_read, \
   .stream_write = string_stream_write, \
   .stream_has_more = string_stream_has_more \
}

#endif // __STRING_STREAM_H__


