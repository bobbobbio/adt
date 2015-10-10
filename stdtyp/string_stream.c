// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <stdtyp/string_stream.h>
#include <stdtyp/string.h>

adt_func_body(string_stream);

const struct stream_interface string_stream_interface = {
   .stream_read = string_stream_read,
   .stream_write = string_stream_write,
   .stream_has_more = string_stream_has_more
};

struct stream *
string_stream_to_stream(struct string_stream *s)
{
   return (struct stream *)s;
}

void
string_stream_init(struct string_stream *s)
{
   s->type = STRING_STREAM;
   s->offset = 0;
   s->string = NULL;
   s->is_const = false;
}

void
string_stream_destroy(struct string_stream *s) {}

struct string_stream
string_stream_make_var(struct string *str)
{
   struct string_stream s = string_stream_make();
   s.string = str;
   return s;
}

struct string_stream
string_stream_const_make_var(const struct string *str)
{
   struct string_stream s = string_stream_make();
   s.string = (struct string *)str;
   s.is_const = true;
   return s;
}

struct error
string_stream_read(struct stream *s, struct string *buff,
   size_t want, size_t *got)
{
   adt_assert(s->type == STRING_STREAM);
   struct string_stream *ss = (struct string_stream *)s;
   adt_assert(ss->string != NULL);

   string_clear(buff);
   size_t to_copy = min(want, string_length(ss->string) - ss->offset);
   string_append_cstring_length(buff,
      &(string_to_cstring(ss->string)[ss->offset]), to_copy);

   ss->offset += to_copy;
   if (got != NULL)
      *got = to_copy;

   return no_error;
}

struct error
string_stream_write(struct stream *s, const struct string *data)
{
   adt_assert(s->type == STRING_STREAM);
   struct string_stream *ss = (struct string_stream *)s;
   adt_assert(!ss->is_const);
   adt_assert(ss->string != NULL);

   string_append_string(ss->string, data);

   return no_error;
}

bool
string_stream_has_more(struct stream *s)
{
   adt_assert(s->type == STRING_STREAM);
   struct string_stream *ss = (struct string_stream *)s;
   adt_assert(ss->string != NULL);

   return ss->offset < string_length(ss->string);
}
