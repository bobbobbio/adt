#ifndef __ARGPARSE_H__
#define __ARGPARSE_H__

#include <adt.h>
#include <stdtyp/map.h>
#include <stdtyp/string.h>

#include <unistd.h>
#include <stdarg.h>

enum arg_type {
   ARG_BOOL,
   ARG_STRING,
   ARG_NUM,
   ARG_STRING_ARRAY,
};

enum arg_needs_type {
   ARG_OPTIONAL,
   ARG_REQUIRED
};

struct arg_template;
adt_func_header(arg_template);

struct arg_value;
adt_func_pod_header(arg_value);

map_gen_header(string_arg_template_map, string, arg_template);
map_gen_header(string_arg_map, string, arg_value);
map_gen_podk_header(char_string_map, char, string);

struct arg_dict {
   struct string_arg_template_map templates;
   struct char_string_map shortcuts;
   struct string_arg_map values;
};
adt_func_header(arg_dict);

void *
get_arg(struct arg_dict *dict, const struct string *cmd);

void
_declare_args(struct arg_dict *ad, const char *arg_name, ...);

#define define_args(...) \
static void declare_args(struct arg_dict *ad) \
{ _declare_args(ad, __VA_ARGS__, NULL); } \
SWALLOWSEMICOLON

void
parse_args(struct arg_dict *dict, char **argv, int argc);

#define arg_main(_arg_dict) \
_arg_call(_arg_dict); \
int main(int argc, char **argv) { \
   create(arg_dict, ad); \
   declare_args(&ad); \
   parse_args(&ad, argv, argc); \
   return _arg_call(&ad); } \
int _arg_call(_arg_dict)

struct arg_entry {
   char sc;
   char *word;
   char *desc;
   struct arg_entry *next;
};

#define define_arg(_sc, _word, _desc) \
arg_list_head = (struct arg_entry *)&{ .tail = arg_tail};

#endif // __ARGPARSE_H__
