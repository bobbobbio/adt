#ifndef __ERROR_H
#define __ERROR_H

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

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

create_error_header(_no_error);

#define no_error error_make(_no_error, "")

#define echeck(expr) \
   do { struct error e = expr; \
   if (e.type != _no_error) error_panic(e, #expr); } while(0)

#define epass(expr) \
   do { struct error e = expr; \
   if (e.type != _no_error) return e; } while(0)

void error_panic(struct error e, char *code);
void panic(char *msg);

#endif // __ERROR_H
