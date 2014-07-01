#include <stdtyp/regex.h>

struct regex
regex_compile(const struct string *s)
{
   struct regex r;
   memset(&r, 0, sizeof(r));

   // Count the number of match groups
   const char *p = string_to_cstring(s);
   int stack = 0;
   for (int i = 0; i < string_length(s); i++) {
      if (p[i] == '(' && (i == 0 || p[i - 1] != '\\'))
         stack++;
      else if (p[i] == ')' && stack > 0 && p[i - 1] != '\\') {
         stack--;
         r.num_groups++;
      }
   }

   int ret = regcomp(&r.r, string_to_cstring(s), REG_EXTENDED | REG_NEWLINE);
   if (ret != 0) {
      char error_message[256];
      regerror(ret, &r.r, error_message, 256);
      panic("Regex compile error: '%s': %s", string_to_cstring(s),
         error_message);
   }

   return r;
}

void
regex_destroy(struct regex *r)
{
   regfree(&r->r);
}

bool
_regex_match(struct regex *r, const struct string *s, ...)
{
   const char *tm = string_to_cstring(s);
   const char *p = tm;
   regmatch_t m[r->num_groups + 1];

   va_list outs;
   va_start(outs, s);

   int num_matches = 0;
   bool more_args = true;
   while (true) {
      if (regexec(&r->r, p, r->num_groups + 1, m, 0))
         break;

      for (int i = 1; i < r->num_groups + 1; i++) {
         int start = 0;
         int finish = 0;

         if (m[i].rm_so != -1) {
            start = m[i].rm_so + (p - tm);
            finish = m[i].rm_eo + (p - tm);
         }

         struct string *so = NULL;
         if (more_args) {
            so = va_arg(outs, struct string *);
            if (so == NULL)
               more_args = false;
            else {
               string_clear(so);
               string_append_cstring_length(so, tm + start, finish - start);
            }
         }
      }
      p += m[0].rm_eo;
      num_matches++;
   }

   return num_matches > 0;
}

bool
regex_match_all(struct regex *r, const struct string *s,
   struct string_vec_vec *matches_out)
{
   const char *tm = string_to_cstring(s);
   const char *p = tm;
   regmatch_t m[r->num_groups + 1];

   int num_matches = 0;
   while (true) {
      if (regexec(&r->r, p, r->num_groups + 1, m, 0))
         break;

      struct string_vec *match = string_vec_vec_grow(matches_out);
      for (int i = 0; i < r->num_groups + 1; i++) {
         struct string *match_g = string_vec_grow(match);
         if (m[i].rm_so == -1)
            continue;

         int start = m[i].rm_so + (p - tm);
         int finish = m[i].rm_eo + (p - tm);

         string_append_cstring_length(match_g, tm + start, finish - start);
      }
      p += m[0].rm_eo;
      num_matches++;
   }

   return num_matches > 0;
}
