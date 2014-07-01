#include <error.h>

create_error_body(_no_error);

void
error_panic(struct error e, char *code)
{
   _panic("Got error: %s : \"%s\" : %s\n", e.type, code, e.msg);
}

void
_panic(char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);
   vfprintf(stderr, fmt, args);
   fprintf(stderr, "\n");
   abort();

   va_end(args);
}

char *
error_msg(struct error e)
{
   return e.msg;
}

void
_assert_msg(bool test, const char *code, char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   if (!test) {
      if (fmt != NULL) {
         fprintf(stderr, "Assert failed: %s : ", code);
         vfprintf(stderr, fmt, args);
         fprintf(stderr, "\n");
      }
      abort();
   }

   va_end(args);
}
