#ifndef __SUBPROCESS_H__
#define __SUBPROCESS_H__

#include <adt.h>
#include <stdtyp/string.h>

create_error_header(subprocess_error);

struct error
subprocess_run(const struct string *, struct string *);

#endif // __SUBPROCESS_H__
