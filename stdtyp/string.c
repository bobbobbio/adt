#include <stdtyp/file.h>
#include <stdtyp/regex.h>
#include <stdtyp/string.h>
#include <stdtyp/tokenizer.h>

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

adt_func_body(string);

vector_gen_body(string_vec, string);
vector_gen_body(string_vec_vec, string_vec);

struct string_vec
_strvw(const char *a, ...)
{
   va_list argp;
   va_start(argp, a);

   struct string_vec out = string_vec_make();

   while (a != NULL) {
      create_const_string(str, a);
      string_vec_append(&out, &str);
      a = va_arg(argp, const char *);
   }

   return out;
}

char *
_printer(struct string_vec *sv, void(*p)(const void *, struct string *),
   const void *v)
{
   struct string *str = string_vec_grow(sv);
   p(v, str);
   return str->buff;
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
   assert(dest->mallocd);

   string_destroy(dest);
   dest->length = src->length;
   dest->buff_len = src->length + 1;
   dest->buff = (char *)malloc(dest->buff_len);
   memcpy(dest->buff, src->buff, dest->buff_len);
}

void
string_clear(struct string *s)
{
   assert(s->mallocd);
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

const char *
string_to_cstring(const struct string *s)
{
   return (const char *)s->buff;
}

uint64_t
string_length(const struct string *s)
{
   return s->length;
}

char
string_char_at_index(const struct string *s, uint64_t i)
{
   assert(i < s->length);
   return s->buff[i];
}

static void
string_expand(struct string *s)
{
   assert(s->mallocd);

   s->buff_len *= 2;
   s->buff = (char *)realloc(s->buff, s->buff_len);
}

static void
_string_append_char(struct string *s, char c)
{
   assert(s->mallocd);
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
string_read_fd(struct string *s, int fd, size_t want)
{
   assert(s->mallocd);
   string_clear(s);

   int bytes_read = 0;
   while (true) {
      size_t space = s->buff_len - s->length;
      size_t to_read;
      if (want > 0)
         to_read = min(space, want);
      else
         to_read = space;
      bytes_read = read(fd, &(s->buff[s->length]), to_read);
      if (bytes_read == 0)
         break;
      if (bytes_read == -1) { // error
         if (errno == EINTR || errno == EAGAIN)
            continue;
         else {
            return error_make(file_read_error, "Failed to read from fd");
         }
      } else { // we actually read data
         s->length += bytes_read;
         want -= bytes_read;
         if (s->length == s->buff_len) {
            string_expand(s);
         }
         s->buff[s->length] = '\0';
      }
   }

   return no_error;
}

void
string_append_int(struct string *s, int i)
{
   assert(s->mallocd);

   char b[20]; // should be long enough to hold any 64 bit int
   sprintf(b, "%d", i);
   string_append_cstring(s, b);
}

int
string_to_int(const struct string *s)
{
   assert(s->length > 0);
   return atoi(s->buff);
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

   create_tokenizer(tkn, s);
   create(string, sc);
   string_append_char(&sc, c);
   tokenizer_set_skip_chars(&tkn, &sc);
   tokenizer_set_allow_empty(&tkn, true);

   create(string, t);
   while (tokenizer_get_next(&tkn, &t))
      string_vec_append(vec_out, &t);
}

void
string_tokenize(const struct string *s, struct string_vec *vec_out)
{
   string_vec_clear(vec_out);

   create_tokenizer(tkn, s);
   create(string, t);
   while (tokenizer_get_next(&tkn, &t))
      string_vec_append(vec_out, &t);
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
   assert(s->mallocd);
   assert(si <= ei);
   assert(si >= 0);
   assert(ei < s->length);

   int nbuff_len = s->buff_len - (ei - si + 1);
   char *nbuff = (char *)malloc(nbuff_len);

   int bi = 0;
   for (int i = 0; i < s->buff_len; i++) {
      if (i < si || i > ei)
         nbuff[bi++] = s->buff[i];
   }
   assert(bi == nbuff_len);

   free(s->buff);
   s->buff = nbuff;
   s->buff_len = nbuff_len;
   s->length = s->buff_len - 1;
}

int
_string_append_format(struct string *s, char *fmt, ...)
{
   va_list args;
   va_list args2;
   va_start(args, fmt);
   va_start(args2, fmt);

   int len = vsnprintf(NULL, 0, fmt, args);
   int fs = s->buff_len - s->length;
   while (len + 1 > fs) {
      string_expand(s);
      fs = s->buff_len - s->length;
   }
   assert(vsnprintf(&s->buff[s->length], fs, fmt, args2) < fs);
   s->length += len;

   va_end(args);
   va_end(args2);

   return len;
}

static bool
_string_replace(struct string *s, const struct regex *r, const struct string *n,
   bool all)
{
   assert(s->mallocd);

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

         assert(m2[1].rm_so != -1);
         char num[10];
         memset(num, 0, 10);
         assert(m2[1].rm_eo - m[1].rm_so < 10);
         strncpy(num, p2 + m2[1].rm_so, m2[1].rm_eo - m2[1].rm_so);
         int gn = atoi(num);
         assert(gn <= r->num_groups && gn >= 1);
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

