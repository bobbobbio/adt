#ifndef __ERROR_H
#define __ERROR_H

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <adt.h>

struct error {
   char *type;
   char *msg;
};

enum error_mode {
   ERROR_PANIC = 0,
   ERROR_PASS = 1
};
extern __thread enum error_mode current_error_mode;

#define create_error_header(name) \
   extern char *name

#define create_error_body(name) \
   char *name = #name

#define error_panic(e, msg) \
   _error_panic(e, msg, __FILE__, __LINE__)

#define eraise(name, message) \
   do { struct error e = { .type = name, .msg = message }; \
      if (current_error_mode == ERROR_PANIC) \
         error_panic(e, #name); \
      return e; \
   } while(0)

#define error_equal(error_a, error_b) \
   (error_a.type == error_b.type)

#define error_is_type(error_a, etype) \
   (error_a.type == etype)

create_error_header(_no_error);

#define ehandle(ename, expr) \
   _ehandle(ename, expr, unq(ponce), unq(old_error_mode))

#define _ehandle(ename, expr, ponce, old_error_mode) \
   for (bool ponce = true; ponce; ) \
   for (enum error_mode old_error_mode = current_error_mode; ponce; ) \
   for (; ponce; current_error_mode = old_error_mode) \
   for (current_error_mode = ERROR_PASS; ponce; ponce = false) \
   for (struct error ename = expr; !error_equal(ename, no_error); \
      ename = no_error) \
   for (current_error_mode = old_error_mode; ponce; ponce = false)

#define epanic(expr) \
   _epanic(expr, unq(old_error_mode), unq(e))

#define _epanic(expr, old_error_mode, e) \
   do { \
      enum error_mode old_error_mode = current_error_mode; \
      current_error_mode = ERROR_PANIC; \
      struct error e = expr; \
      if (e.type != _no_error) \
         error_panic(e, #expr); \
      current_error_mode = old_error_mode; \
   } while(0)

#define no_error ((struct error){ .type = _no_error, .msg = "" })

#define ereraise(expr) \
   _reraise(expr, unq(e))

#define _reraise(expr, e) \
   do { struct error e = expr; \
      if (e.type != _no_error) { \
         if (current_error_mode == ERROR_PANIC) \
            error_panic(e, #expr); \
         return e; \
      } \
   } while(0)

#define panic(...) \
   adt_print(_panic, __VA_ARGS__)

#define adt_assert(...) \
   __adt_assert(__VA_ARGS__, NULL)
#define __adt_assert(test, ...) \
   adt_print(_adt_assert, test, #test, __FILE__, __LINE__, __VA_ARGS__)

#define noop_error(...) no_error

void _error_panic(struct error e, char *code, const char *file, int line);
void _panic(char *fmt, ...);
char *error_msg(struct error e);
void _adt_assert(
   bool test, const char *code, const char *file, int line, char *fmt, ...);
void print_backtrace(int skip_frames);

#endif // __ERROR_H
