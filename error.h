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

#define create_error_header(name) \
   extern char *name

#define create_error_body(name) \
   char *name = #name

#define error_make(name, message) \
   ((struct error){ .type = name, .msg = message })

#define error_equal(error_a, error_b) \
   (error_a.type == error_b.type)

#define error_is_type(error_a, etype) \
   (error_a.type == etype)

create_error_header(_no_error);

#define ehandle(ename, expr) \
   for (struct error ename = expr; !error_equal(ename, no_error); \
   ename = no_error)

#define no_error error_make(_no_error, "")

#define echeck(expr) \
   do { struct error e = expr; \
   if (e.type != _no_error) error_panic(e, #expr); } while(0)

#define epass(expr) \
   do { struct error e = expr; \
   if (e.type != _no_error) return e; } while(0)

#define panic(...) \
   adt_print(_panic, __VA_ARGS__)
#define assert_msg(test, ...) \
   adt_print(_assert_msg, test, #test, __VA_ARGS__)

#define noop_error(...) no_error

void error_panic(struct error e, char *code);
void _panic(char *fmt, ...);
char *error_msg(struct error e);
void _assert_msg(bool test, const char *code, char *fmt, ...);

#endif // __ERROR_H
