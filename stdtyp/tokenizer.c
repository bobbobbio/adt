#include <stdtyp/tokenizer.h>
#include <stdtyp/stream.h>

adt_func_body(tokenizer);

#define READ_SIZE 4096

void tokenizer_init(struct tokenizer *t)
{
   t->pos = 0;
   t->allow_empty = false;
   string_init(&t->buff);
   string_init(&t->skip_chars);
   string_copy(&t->skip_chars, strw(" \t\r\n"));
}

void tokenizer_destroy(struct tokenizer *t)
{
   string_destroy(&t->buff);
   string_destroy(&t->skip_chars);
}

struct tokenizer
tokenizer_make_var(struct stream *s)
{
   struct tokenizer t = tokenizer_make();
   tokenizer_set_stream(&t, s);
   return t;
}

struct tokenizer *
tokenizer_new_var(struct stream *s)
{
   struct tokenizer *t = tokenizer_new();
   tokenizer_set_stream(t, s);
   return t;
}

void
tokenizer_set_stream(struct tokenizer *t, struct stream *s)
{
   t->stream = s;
   t->pos = 0;
}

static bool
is_skipchar(struct tokenizer *t, char c)
{
   return string_contains_char(&t->skip_chars, c);
}

static void
_tokenizer_read(struct tokenizer *t)
{
   size_t bytes_read;
   stream_read_n_or_less(t->stream, &t->buff, READ_SIZE, &bytes_read);
   t->pos = 0;
}


static void
_tokenizer_skip(struct tokenizer *t)
{
   while (is_skipchar(t, string_char_at_index(&t->buff, t->pos))) {
      t->pos++;
      if (t->pos >= string_length(&t->buff)) {
         if (!stream_has_more(t->stream))
            break;
         else
            _tokenizer_read(t);
      }
   }
}

static bool
non_empty_get_next(struct tokenizer *t, struct string *s)
{
   adt_assert(t->stream != NULL);

   while (t->pos >= string_length(&t->buff)) {
      if (!stream_has_more(t->stream)) {
         return false;
      } else
         _tokenizer_read(t);
   }

   _tokenizer_skip(t);

   string_clear(s);
   while (t->pos < string_length(&t->buff)) {
      char c = string_char_at_index(&t->buff, t->pos);
      if (is_skipchar(t, c))
         break;
      string_append_char(s, c);
      t->pos++;

      if (t->pos >= string_length(&t->buff))
         _tokenizer_read(t);
   }

   return true;
}

static bool
empty_get_next(struct tokenizer *t, struct string *s)
{
   adt_assert(t->stream != NULL);

   while (t->pos >= string_length(&t->buff)) {
      if (!stream_has_more(t->stream))
         return false;
      else
         _tokenizer_read(t);
   }

   string_clear(s);
   while (t->pos < string_length(&t->buff)) {
      char c = string_char_at_index(&t->buff, t->pos);
      if (is_skipchar(t, c))
         break;
      string_append_char(s, c);
      t->pos++;

      if (t->pos >= string_length(&t->buff))
         _tokenizer_read(t);
   }

   t->pos++;

   return true;
}

bool
tokenizer_get_next(struct tokenizer *t, struct string *s)
{
   if (t->allow_empty)
      return empty_get_next(t, s);
   else
      return non_empty_get_next(t, s);
}

void
tokenizer_set_skip_chars(struct tokenizer *t, struct string *s)
{
   string_copy(&t->skip_chars, s);
}

void
tokenizer_set_allow_empty(struct tokenizer *t, bool ae)
{
   t->allow_empty = ae;
}
