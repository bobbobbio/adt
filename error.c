// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <error.h>
#include <stdtyp/file.h>
#include <execinfo.h>
#include <stdtyp/threading.h>

create_error_body(file_not_found_error);
create_error_body(file_access_error);
create_error_body(file_out_of_space_error);
create_error_body(file_already_exists_error);
create_error_body(errno_unknown_error);

// We panic by default
__thread enum error_mode current_error_mode = ERROR_PANIC;

create_error_body(_no_error);

static struct expected_assert *g_expected_assert = NULL;

adt_func_body(expected_assert);

void
expected_assert_init(struct expected_assert *self)
{
   set_expected_assert(self);
}

void
expected_assert_print(const struct expected_assert *self, struct string *str)
{
   string_append_cstring(str, "assert ");

   switch (self->type) {
      case EXPECTED_ASSERT_MESSAGE:
         string_append_format(str, "message: %s", self->expected_message);
      break;
      case EXPECTED_ASSERT_CONDITION:
         string_append_format(str, "condition: %s", self->expected_condition);
      break;
   }
}

void
expected_assert_destroy(struct expected_assert *self)
{
   adt_assert(self == g_expected_assert);
   clear_expected_assert();

   panic("Expected %s", print(expected_assert, self));
}

struct expected_assert *
expected_assert_message_create(const char *message)
{
   struct expected_assert *self = expected_assert_new();
   self->expected_message = message;
   self->type = EXPECTED_ASSERT_MESSAGE;
   return self;
}

struct expected_assert *
expected_assert_condition_create(const char *condition)
{
   struct expected_assert *self = expected_assert_new();
   self->expected_condition = condition;
   self->type = EXPECTED_ASSERT_CONDITION;
   return self;
}

void
expected_assert_handle(
   struct expected_assert *self,
   const struct string *condition,
   const struct string *message)
{
   clear_expected_assert();

   adt_assert(self != NULL);

   switch (self->type) {
      case EXPECTED_ASSERT_MESSAGE:
         adt_assert_equal(string,
            strw(self->expected_message), message);
      break;
      case EXPECTED_ASSERT_CONDITION:
         adt_assert_equal(string,
            strw(self->expected_condition), condition);
      break;
   }
}

void
set_expected_assert(struct expected_assert *e)
{
   g_expected_assert = e;
}

void
clear_expected_assert(void)
{
   g_expected_assert = NULL;
}

static struct mutex g_assert_handler_lock = mutex_initializer;

void
_assert(const char *code, const char *file, int line, char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   if (g_expected_assert) {
      create(string, msg);
      string_append_format_va_list(&msg, fmt, args);
      with_mutex(&g_assert_handler_lock)
         expected_assert_handle(g_expected_assert, strw(code), &msg);
      exit(EXIT_SUCCESS);
   } else {
      print_backtrace(2);
      vfprintf(stderr, fmt, args);
      fprintf(stderr, ": %s : in file %s on line %d\n", code, file, line);
      abort();
   }

   va_end(args);
}

char *
error_msg(struct error e)
{
   return e.msg;
}

void
_adt_assert(const char *code, const char *file, int line, char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   create(string, msg);
   if (fmt != NULL) {
      string_append_format_va_list(&msg, fmt, args);
   }

   _assert(code, file, line, "assert: %s", string_to_cstring(&msg));

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
      backtrace_symbols_fd(&call_stack[skip_frames],
         call_stack_size - skip_frames, STDERR_FILENO);
   }
}

adt_printer_body(error);

void
error_print(const struct error *e, struct string *s)
{
   string_append_format(s, "%s(\"%s\")", e->type, e->msg);
}

char *
errno_to_error(void)
{
   if (errno == EACCES)
      return file_access_error;
   else if (errno == ENOENT)
      return file_not_found_error;
   else if (errno == EEXIST)
      return file_already_exists_error;
   else if (errno == ENOSPC)
      return file_out_of_space_error;
   else
      return errno_unknown_error;
}
