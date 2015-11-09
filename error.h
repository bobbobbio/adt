// Copyright 2015 Remi Bernotavicius. All right reserved.

#ifndef __ERROR_H
#define __ERROR_H

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <adt.h>

/*
 * This file contains functions for dealing with errors.
 *
 * When a function needs to throw an error, it must have struct error as it's
 * return type. Like follows:
 *
 * struct error
 * do_file_io(void)
 * {
 *    if (bad_things())
 *       eraise(file_read_error, "Something has gone wrong...");
 *    else
 *       return no_error;
 * }
 *
 * A good pattern to use is to use the warn_unused_result attribute with this
 * function like follows when declaring the function:
 *
 * struct error
 * do_file_io(void)
 *    a_warn_unused_result;
 *
 * This tries to ensure that error don't get lost.
 *
 * When calling a function that may raise an error, there are 3 options for what
 * to do wit that error.  If that function wants the error to just crash, it can
 * do:
 *    ecrash(do_file_io());
 *
 * If it wants to handle the error it can do this:
 *    ehandle (error, do_file_io()) {
 *       // handle the error here.
 *    }
 *
 * You can use the following pattern to filter errors:
 *    ehandle (error, do_file_io()) {
 *       if (error_is_type(error, transient_error)) {
 *          printf("WARN: Got a transient error\n")
 *       } else
 *          error_panic(error);
 *    }
 *
 * It will ignore errors of type transient_error, and crash if it got some other
 * error type.
 *
 * If the calling function itself returns an error, it can use the ereraise
 * macro to raise the potential error as if this function was raising an error
 * itself.
 *
 * struct error
 * do_file_io(void)
 * {
 *    ereraise(other_function());
 *
 *    return no_error;
 * }
 *
 * The following function just passes on errors that other_function may raise,
 * and the caller of do_file_io now has to deal with the error.
 *
 * Note that you can use the ehandle in conjunction with ereraise to filter the
 * errors that get passed out like so:
 *    ehandle (error, do_file_io()) {
 *       if (error_is_type(error, transient_error)) {
 *          printf("WARN: Got a transient error\n")
 *       } else
 *          ereraise(error);
 *    }
 *
 * Be careful using this pattern so as to not loose errors.  If you forget to
 * either reraise or crash at the end of an ehandle, it may just swallow an
 * error.
 *
 */

struct error {
   char *type;
   char *msg;
};

enum error_mode {
   ERROR_PANIC = 0,
   ERROR_PASS = 1
};
extern __thread enum error_mode current_error_mode;

enum expected_assert_type {
   EXPECTED_ASSERT_MESSAGE = 0,
   EXPECTED_ASSERT_CONDITION = 1
};

struct expected_assert {
   union {
      const char *expected_message;
      const char *expected_condition;
   };
   enum expected_assert_type type;
};
adt_func_header(expected_assert);

adt_printer_header(error);

void
error_print(const struct error *, struct string *);

void
set_expected_assert(struct expected_assert *);

void
clear_expected_assert(void);

struct expected_assert *
expected_assert_message_create(const char *message);

struct expected_assert *
expected_assert_condition_create(const char *condition);

#define with_expected_assert_condition(cond) \
   ctx_def(struct expected_assert *unq(n) a_unused \
      a_cleanup(expected_assert_freer) = \
      expected_assert_condition_create(cond))

#define with_expected_assert_message(cond) \
   ctx_def(struct expected_assert *unq(n) a_unused \
      a_cleanup(expected_assert_freer) = \
      expected_assert_message_create(cond))

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

#define ecrash(expr) \
   _ecrash(expr, unq(old_error_mode), unq(e))

#define _ecrash(expr, old_error_mode, e) \
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

#define unlikely(x) \
   __builtin_expect(!!(x), 0)

// This is like a regular assert except that you can add an optional message as
// the second argument, and it accepts adt printing (see string.h).
#define adt_assert(...) \
   __adt_assert(__VA_ARGS__, NULL)
#define __adt_assert(test, ...)                                                \
   if (unlikely(!(test)))                                                      \
      adt_print(_adt_assert, #test, __FILE__, __LINE__, __VA_ARGS__)

#ifdef ASSERTS_OFF
#undef __adt_assert
#define __adt_assert(...)
#endif

#define adt_assert_size(type, cont, size) \
   adt_assert(type##_size((cont)) == size, \
      "size of %s is not %d", print(type, (cont)), size)

#define adt_assert_equal(type, a, b) \
   adt_assert(type##_equal((a), (b)), \
      "%s != %s", print(type, (a)), print(type, (b)))

void _error_panic(struct error e, char *code, const char *file, int line)
   a_noreturn;
void _panic(char *fmt, ...)
   a_format(printf, 1, 2) a_noreturn;
char *error_msg(struct error e);
void _adt_assert(const char *code, const char *file, int line, char *fmt, ...);
void print_backtrace(int skip_frames);

#endif // __ERROR_H
