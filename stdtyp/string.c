// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <stdtyp/file.h>
#include <stdtyp/regex.h>
#include <stdtyp/string.h>
#include <stdtyp/string_stream.h>

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

adt_func_body(string);

vector_gen_body(string_vec, string);
vector_gen_body(string_vec_vec, string_vec);

void
_string_vec_append_cstrs(struct string_vec *self, const char *a, ...)
{
   va_list argp;
   va_start(argp, a);

   while (a != NULL) {
      create_const_string(str, a);
      string_vec_append(self, &str);
      a = va_arg(argp, const char *);
   }

   va_end(argp);
}

void
_string_vec_append_strings(struct string_vec *self,
   const struct string *a, ...)
{
   va_list argp;
   va_start(argp, a);

   while (a != NULL) {
      string_vec_append(self, a);
      a = va_arg(argp, const struct string *);
   }

   va_end(argp);
}

void
carr_freer(const char ***c)
{
   free(*c);
}

struct string
string_make_var(const char *cstr)
{
   struct string a = string_make();
   string_append_cstring(&a, cstr);
   return a;
}

struct string *
string_new_var(const char *cstr)
{
   struct string *a = string_new();
   string_append_cstring(a, cstr);
   return a;
}

struct string
string_const_make(const char *buff)
{
   struct string s;
   s.buff = (char *)buff;
   s.buff_len = strlen(buff) + 1;
   s.length = strlen(buff);
   s.mallocd = false;

   return s;
}

const struct string *
string_const_new(const char *buff)
{
   struct string *s = malloc(sizeof(struct string));
   s->buff = (char *)buff;
   s->buff_len = strlen(buff) + 1;
   s->length = strlen(buff);
   s->mallocd = false;

   return s;
}

void
string_init(struct string *s)
{
   s->buff_len = 10;
   s->buff = (char *)malloc(s->buff_len);
   s->buff[0] = '\0';
   s->length = 0;
   s->mallocd = true;
}

void
string_destroy(struct string *s)
{
   if (s->mallocd)
      free(s->buff);
}

void
string_copy(struct string *dest, const struct string *src)
{
   adt_assert(dest->mallocd);

   string_destroy(dest);
   dest->length = src->length;
   dest->buff_len = src->length + 1;
   dest->buff = (char *)malloc(dest->buff_len);
   memcpy(dest->buff, src->buff, dest->buff_len);
}

void
string_clear(struct string *s)
{
   adt_assert(s->mallocd);
   s->length = 0;
   s->buff[0] = '\0';
}

void
string_set_cstring(struct string *s, char *cstr)
{
   string_clear(s);
   string_append_cstring(s, cstr);
}

void
string_print(const struct string *s, struct string *d)
{
   string_append_string(d, s);
}

static void
string_expand(struct string *s)
{
   adt_assert(s->mallocd);

   s->buff_len *= 2;
   s->buff = (char *)realloc(s->buff, s->buff_len);
}

static void
_string_append_char(struct string *s, char c)
{
   adt_assert(s->mallocd);
   s->length++;
   if (s->length + 1 > s->buff_len)
      string_expand(s);
   s->buff[s->length - 1] = c;
}

void
string_append_char(struct string *s, char c)
{
   _string_append_char(s, c);
   s->buff[s->length] = '\0';
}

void
string_append_cstring(struct string *s, const char *cstr)
{
   for (const char *c = cstr; *c != '\0'; c++)
      _string_append_char(s, *c);
   s->buff[s->length] = '\0';
}

void
string_append_cstring_length(struct string *s, const char *cstr, size_t len)
{
   for (int i = 0; i < len; i++)
      _string_append_char(s, cstr[i]);
   s->buff[s->length] = '\0';
}

void
string_append_string(struct string *s, const struct string *a)
{
   string_append_cstring(s, string_to_cstring(a));
}

struct error
string_read_fd(struct string *s, int fd, size_t want, size_t *got, bool *done)
{
   adt_assert(s->mallocd);
   string_clear(s);

   *done = false;

   int bytes_read = 0;
   int total_bytes_read = 0;
   while (true) {
      size_t space = s->buff_len - s->length;
      size_t to_read;
      if (want > 0)
         to_read = min(space, want);
      else
         to_read = space;
      bytes_read = read(fd, &(s->buff[s->length]), to_read);
      total_bytes_read += bytes_read;
      if (bytes_read == 0) {
         *done = true;
         break;
      }
      if (bytes_read == -1) { // error
         if (errno == EINTR || errno == EAGAIN)
            continue;
         else {
            eraise_errno_error();
         }
      } else { // we actually read data
         s->length += bytes_read;
         want -= bytes_read;
         if (s->length == s->buff_len) {
            string_expand(s);
         }
         s->buff[s->length] = '\0';
         if (bytes_read < to_read && got != NULL)
            break;
      }
   }

   if (got != NULL)
      *got = total_bytes_read;

   return no_error;
}

void
string_append_int(struct string *s, int i)
{
   adt_assert(s->mallocd);

   char b[20]; // should be long enough to hold any 64 bit int
   sprintf(b, "%d", i);
   string_append_cstring(s, b);
}

int
string_to_int(const struct string *s)
{
   adt_assert(s->length > 0);
   return atoi(s->buff);
}

void
string_strip(struct string *s)
{
   if (string_length(s) == 0)
      return;

   unsigned si = 0;
   unsigned ei = string_length(s) - 1;
   while (char_is_whitespace(string_char_at_index(s, si))) {
      si++;
      if (si >= string_length(s)) {
         string_clear(s);
         return;
      }
   }
   while (char_is_whitespace(string_char_at_index(s, ei)))
      ei--;

   if (ei < string_length(s) - 1)
      string_remove_substring(s, ei + 1, string_length(s) - 1);
   if (si > 0)
      string_remove_substring(s, 0, si - 1);
}

bool
string_equal(const struct string *a, const struct string *b)
{
   return strcmp(a->buff, b->buff) == 0;
}

void
string_split(const struct string *s, char c, struct string_vec *vec_out)
{
   string_vec_clear(vec_out);
   unsigned st = 0;
   for (unsigned i = 0; i < string_length(s); i++) {
      if (string_char_at_index(s, i) == c) {
         struct string *t = string_vec_grow(vec_out);
         string_append_cstring_length(t, &s->buff[st], i - st);
         st = i + 1;
      }
   }

   struct string *t = string_vec_grow(vec_out);
   string_append_cstring_length(t, &s->buff[st], string_length(s) - st);
}

void
string_tokenize(const struct string *s, struct string_vec *vec_out)
{
   string_vec_clear(vec_out);

   unsigned si = 0;
   unsigned ei = 0;

   for (; ei < string_length(s); ei++) {
      if (char_is_whitespace(string_char_at_index(s, ei))) {
         if (ei - si > 0) {
            struct string *sc = string_vec_grow(vec_out);
            string_append_cstring_length(sc, &s->buff[si], ei - si);
         }
         si = ei + 1;
      }
   }
   struct string *sc = string_vec_grow(vec_out);
   string_append_cstring_length(sc, &s->buff[si], ei - si);
}

bool
string_starts_with(const struct string *a, const struct string *b)
{
   if (b->length > a->length)
      return false;

   for (int i = 0; i < b->length; i++) {
      if (string_char_at_index(a, i) != string_char_at_index(b, i))
         return false;
   }

   return true;
}

bool
string_ends_with(const struct string *a, const struct string *b)
{
   if (a->length < b->length)
      return false;

   int len_diff = a->length - b->length;
   for (int i = 0; i < b->length; i++) {
      if (string_char_at_index(a, len_diff + i) != string_char_at_index(b, i))
         return false;
   }

   return true;
}

bool
string_contains_char(const struct string *s, char c)
{
   for (unsigned i = 0; i < s->length; i++)
      if (string_char_at_index(s, i) == c)
         return true;

   return false;
}

bool
string_contains_substring(const struct string *a, const struct string *b)
{
   if (a->length < b->length)
      return false;

   for (unsigned i = 0; i <= a->length - b->length; i++) {
      for (unsigned j = 0; j < b->length; j++) {
         if (string_char_at_index(a, i + j) != string_char_at_index(b, j))
            break;
         else if (j == b->length - 1)
            return true;
      }
   }

   return false;
}

int
string_compare(const struct string *a, const struct string *b)
{
   return strcmp(string_to_cstring(a), string_to_cstring(b));
}

uint64_t
string_hash(const struct string *s)
{
   uint64_t hash = 0;
   uint64_t exp = 1;
   const char *str = string_to_cstring(s);

   while (*str != '\0') {
      hash += exp * *str;
      exp *= 37;

      str++;
   }

   return hash;
}

void
string_vec_join(struct string *d, const struct string_vec *v, char s)
{
   string_clear(d);

   if (string_vec_size(v) == 0)
      return;

   string_append_string(d, string_vec_at(v, 0));

   for (int i = 1; i < string_vec_size(v); i++) {
      string_append_char(d, s);
      string_append_string(d, string_vec_at(v, i));
   }
}

void
string_remove_substring(struct string *s, int si, int ei)
{
   adt_assert(s->mallocd);
   adt_assert(si <= ei);
   adt_assert(si >= 0);
   adt_assert(ei < s->length);

   int nbuff_len = s->length - (ei - si + 1) + 1;
   char *nbuff = (char *)malloc(nbuff_len);

   int bi = 0;
   for (int i = 0; i < s->length; i++) {
      if (i < si || i > ei)
         nbuff[bi++] = s->buff[i];
   }
   adt_assert(bi == nbuff_len - 1);

   free(s->buff);
   s->buff = nbuff;
   s->buff_len = nbuff_len;
   s->length = s->buff_len - 1;
   s->buff[s->length] = '\0';
}

int
string_append_format_va_list(struct string *s, const char *fmt, va_list args)
{
   va_list args_copy;
   va_copy(args_copy, args);

   int len = vsnprintf(NULL, 0, fmt, args_copy);
   int fs = s->buff_len - s->length;
   while (len + 1 > fs) {
      string_expand(s);
      fs = s->buff_len - s->length;
   }
   int res a_unused = vsnprintf(&s->buff[s->length], fs, fmt, args);
   adt_assert(res < fs);
   s->length += len;

   va_end(args_copy);

   return len;
}

int
_string_append_format(struct string *s, const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   int len = string_append_format_va_list(s, fmt, args);

   va_end(args);
   return len;
}

static bool
_string_replace(struct string *s, const struct regex *r, const struct string *n,
   bool all)
{
   adt_assert(s->mallocd);

   char *tm = s->buff;
   string_init(s);

   create_regex(ce_reg, strw("\\$([0-9]+)"));

   const char *p = tm;
   regmatch_t m[r->num_groups + 1];

   int num_matches = 0;
   while (true) {
      if (regexec(&r->r, p, r->num_groups + 1, m, 0))
         break;

      create(string, rep);

      const char *p2 = n->buff;
      regmatch_t m2[3];
      while (true) {
         if (regexec(&ce_reg.r, p2, 3, m2, 0))
            break;

         adt_assert(m2[1].rm_so != -1);
         char num[10];
         memset(num, 0, 10);
         adt_assert(m2[1].rm_eo - m[1].rm_so < 10);
         strncpy(num, p2 + m2[1].rm_so, m2[1].rm_eo - m2[1].rm_so);
         int gn = atoi(num);
         adt_assert(gn <= r->num_groups && gn >= 1);
         bool escape = m2[0].rm_so > 0 && p2[m2[0].rm_so - 1] == '\\';

         if (m[gn].rm_so != -1 && !escape) {
            string_append_cstring_length(&rep, p2, m2[0].rm_so);
            string_append_cstring_length(&rep, p + m[gn].rm_so,
               m[gn].rm_eo - m[gn].rm_so);
         } else {
            if (!escape)
               string_append_cstring_length(&rep, p2, m2[0].rm_eo);
            else {
               string_append_cstring_length(&rep, p2, m2[0].rm_so - 1);
               string_append_cstring_length(&rep, p2 + m2[0].rm_so,
                  m2[0].rm_eo - m2[0].rm_so);
            }
         }

         p2 += m2[0].rm_eo;
      }
      string_append_cstring(&rep, p2);

      string_append_cstring_length(s, p, m[0].rm_so);
      string_append_string(s, &rep);

      p += m[0].rm_eo;
      num_matches++;

      if (!all)
         break;
   }

   string_append_cstring(s, p);

   free(tm);

   return num_matches > 0;
}

bool
string_replace(struct string *s, const struct regex *r, const struct string *n)
{
   return _string_replace(s, r, n, true);
}

bool
string_replace_first(struct string *s, const struct regex *r,
   const struct string *n)
{
   return _string_replace(s, r, n, false);
}

