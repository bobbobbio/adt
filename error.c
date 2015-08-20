// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <error.h>
#include <stdtyp/file.h>
#include <execinfo.h>

// We panic by default
__thread enum error_mode current_error_mode = ERROR_PANIC;

create_error_body(_no_error);

void
_error_panic(struct error e, char *code, const char *file, int line)
{
   print_backtrace(2);
   _panic("Got error: %s : \"%s\" : %s : in file %s on line %d\n",
      e.type, code, e.msg, file, line);
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
_adt_assert(bool test, const char *code, const char *file,
   int line, char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   if (!test) {
      print_backtrace(2);
      fprintf(stderr, "Assert failed: %s : ", code);
      if (fmt != NULL) {
         vfprintf(stderr, fmt, args);
         fprintf(stderr, " : ");
      }
      fprintf(stderr, "in file %s on line %d", file, line);
      fprintf(stderr, "\n");
      abort();
   }

   va_end(args);
}

void
print_backtrace(int skip_frames)
{
   void *call_stack[1000];
   int call_stack_size = backtrace(call_stack, 1000);
   if (call_stack_size <= 0)
      fprintf(stderr, "Failed to get backtrace");
   else {
      backtrace_symbols_fd(
         &call_stack[skip_frames], call_stack_size - skip_frames, STDERR_FILENO);
   }
}
