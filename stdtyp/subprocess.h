// Copyright 2015 Remi Bernotavicius. All right reserved.

#ifndef __SUBPROCESS_H__
#define __SUBPROCESS_H__

#include <adt.h>
#include <stdtyp/string.h>

create_error_header(subprocess_error);

void
subprocess_exec(const struct string_vec *arg_vec)
   a_noreturn;

pid_t
subprocess_fork(void);

struct error
subprocess_run(const struct string_vec *, struct string *)
   a_warn_unused_result;

#endif // __SUBPROCESS_H__
