#include <stdtyp/regex.h>

struct regex
regex_compile(const struct string *s)
{
   struct regex r;
   memset(&r, 0, sizeof(r));

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
   regmatch_t m[256];

   va_list outs;
   va_start(outs, s);

   int num_matches = 0;
   bool more_args = true;
   while (true) {
      if (regexec(&r->r, p, 256, m, 0))
         break;

      for (int i = 1; i < 256; i++) {
         if (m[i].rm_so == -1)
            break;

         int start = m[i].rm_so + (p - tm);
         int finish = m[i].rm_eo + (p - tm);

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
