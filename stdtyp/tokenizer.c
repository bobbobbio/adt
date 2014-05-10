#include <stdtyp/tokenizer.h>

adt_func_body(tokenizer);

void tokenizer_init(struct tokenizer *t)
{
   t->data = string_new();
   t->pos = 0;
   t->allow_empty = false;
   string_init(&t->skip_chars);
   string_copy(&t->skip_chars, strw(" \t\r\n"));
}

void tokenizer_destroy(struct tokenizer *t)
{
   string_free(t->data);
   string_destroy(&t->skip_chars);
}

struct tokenizer
tokenizer_make_var(const struct string *s)
{
   struct tokenizer t = tokenizer_make();
   tokenizer_set_string(&t, s);
   return t;
}

struct tokenizer *
tokenizer_new_var(const struct string *s)
{
   struct tokenizer *t = tokenizer_new();
   tokenizer_set_string(t, s);
   return t;
}

void
tokenizer_set_string(struct tokenizer *t, const struct string *s)
{
   string_copy(t->data, s);
   t->pos = 0;
}

static bool
is_skipchar(struct tokenizer *t, char c)
{
   return string_contains_char(&t->skip_chars, c);
}

static void
_tokenizer_skip(struct tokenizer *t)
{
   while (t->pos < string_length(t->data)
      && is_skipchar(t, string_char_at_index(t->data, t->pos)))
      t->pos++;
}

static bool
non_empty_get_next(struct tokenizer *t, struct string *s)
{
   assert(t->data != NULL);

   _tokenizer_skip(t);

   if (t->pos >= string_length(t->data))
      return false;

   string_clear(s);
   while (t->pos < string_length(t->data)) {
      char c = string_char_at_index(t->data, t->pos);
      if (is_skipchar(t, c))
         break;
      string_append_char(s, c);
      t->pos++;
   }

   return true;
}

static bool
empty_get_next(struct tokenizer *t, struct string *s)
{
   assert(t->data != NULL);

   if (t->pos > string_length(t->data))
      return false;

   string_clear(s);
   while (t->pos <= string_length(t->data)) {
      if (t->pos == string_length(t->data)) 
         break;
      char c = string_char_at_index(t->data, t->pos);
      if (is_skipchar(t, c))
         break;
      string_append_char(s, c);
      t->pos++;
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

bool
tokenizer_has_next(struct tokenizer *t)
{
   assert(t->data != NULL);

   if (t->allow_empty && t->pos < string_length(t->data))
      return true;

   _tokenizer_skip(t);

   return (t->pos < string_length(t->data));
}

void
tokenizer_set_skip_chars(struct tokenizer *t, const struct string *s)
{
   string_copy(&t->skip_chars, s);
}

void
tokenizer_set_allow_empty(struct tokenizer *t, bool ae)
{
   t->allow_empty = ae;
}
