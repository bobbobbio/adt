#ifndef __ARGPARSE_H__
#define __ARGPARSE_H__

#include <adt.h>
#include <stdtyp/map.h>
#include <stdtyp/string.h>

struct argparse_arg;
map_gen_header(string_arg_map, string, argparse_arg);
map_gen_podk_header(char_string_map, char, string);

struct argparse;
adt_func_header(argparse);

void
argparse_add_required_arg(struct argparse *, struct string *, char,
   struct string *);

void
argparse_add_optional_arg(struct argparse *, struct string *, char,
   struct string *);

void
argparse_add_required_value_arg(struct argparse *, struct string *, char,
   struct string *);

void
argparse_add_optional_value_arg(struct argparse *, struct string *, char,
   struct string *);

void
argparse_add_required_program_arg(struct argparse *, struct string *);

void
argparse_add_optional_program_arg(struct argparse *, struct string *);

bool
argparse_got_arg(struct argparse *, struct string *);

void
argparse_arg_value(struct argparse *, struct string *, struct string *);

void
argparse_program_arg_value(struct argparse *, struct string *);

void
argparse_parse(struct argparse *, int, char **);

#endif // __ARGPARSE_H__
