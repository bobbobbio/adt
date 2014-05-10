#include <stdtyp/argparse.h>

#if 0

struct argparse_arg {
   struct string description;
   bool required;
   bool needs_value;
   char sc;

   bool provided;
   struct string value;
};
adt_func_static(argparse_arg);

map_gen_body(string_arg_map, string, argparse_arg);
map_gen_podk_body(char_arg_map, char, argparse_arg);

struct argparse {
   struct string_arg_map args;
   struct char_string_map args_sc;
   struct argparse_arg *program_arg;
};
adt_func_body(argparse);

void
argparse_init(struct argparse *ap)
{
   string_arg_map_init(&ap->args);
   char_string_map_init(&ap->args_sc);
   ap->program_arg = NULL;
}

void
argparse_destroy(struct argparse *ap)
{
   string_arg_map_destroy(&ap->args);
   char_string_map_destroy(&ap->args_sc);
   if (ap->program_arg != NULL)
      argparse_arg_free(ap->program_arg);
}

// Adding args

static void
_add_arg(struct argparse *ap, struct string *name, char sc, struct string *d,
   bool required, bool needs_value)
{
   create(argparse_arg, arg);
   string_copy(&arg.description, d);
   arg.sc = sc;
   arg.required = required;
   arg.needs_value = needs_value;

   char_arg_map_insert(&ap->args_sc, sc, d);
   string_arg_map_insert(&ap->args, name, &arg);
}

void
argparse_add_required_arg(struct argparse *ap, struct string *n, char sc,
   struct string *d)
{
   _add_arg(ap, n, sc, d, true, false);
}

void
argparse_add_optional_arg(struct argparse *ap, struct string *n, char sc,
   struct string *d)
{
   _add_arg(ap, n, sc, d, false, false);
}

void
argparse_add_required_value_arg(struct argparse *ap, struct string *n, char sc,
   struct string *d)
{
   _add_arg(ap, n, sc, d, true, true);
}

void
argparse_add_optional_value_arg(struct argparse *ap, struct string *n, char sc,
   struct string *d);
{
   _add_arg(ap, n, sc, d, true, false);
}

void
argparse_add_required_program_arg(struct argparse *ap, struct string *d)
{
   assert(ap->program_arg == NULL);
   
   ap->program_arg = program_arg_new();
   string_copy(&ap->program_arg->description, d);
   arg.required = true;
}

void
argparse_add_optional_program_arg(struct argparse *ap, struct string *d)
{
   assert(ap->program_arg == NULL);
   
   ap->program_arg = program_arg_new();
   string_copy(&ap->program_arg->description, d);
   arg.required = false;
}

bool
argparse_got_arg(struct argparse *ap, struct string *n)
{
   create(argparse_arg, arg);
   string_arg_map_get(&ap->args, n, &arg);

   return arg.provided;
}

bool
argparse_got_program_arg(struct argparse *ap)
{
   return ap->program_arg != NULL && ap->program_arg->provided;
}

void
argparse_arg_value(struct argparse *ap, struct string *n, struct string *v_out)
{
   create(argparse_arg, arg);
   string_arg_map_get(&ap->args, n, &arg);

   string_copy(v_out, &arg.value);
}

void
argparse_program_arg_value(struct argparse *ap, struct string *v_out)
{
   string_copy(v_out, &arg.value);
}


static void
invalid_option(const char *n, char o)
{
   fprintf(stderr, "%s: invalid option -- '%c'\n", n, o);
   fprintf(stderr, "Try '%s --help' for more information.\n", n);
}

static struct argparse_arg *
lookup_arg_by_name(struct argparse *ap, struct string *name)
{
   return string_arg_map_at(&ap->args, name);
}

static struct argparse_arg *
lookup_arg_by_sc(struct argparse *ap, char sc)
{
   struct string *name = char_string_map_at(&ap->args_sc, sc);
   if (name == NULL)
      return NULL;
   else
      return lookup_arg_by_name(ap, name);
}

void
argparse_parse(struct argparse *ap, int argc, char **argv)
{
   struct argparse_arg *arg_ent = NULL;
   for (int i = 1; i < argc; i++) {
      char *ent = argv[i];
      if (ent[0] == '-' && ent[1] == '-') {
         handle_arg(ap, strw(&argv[i]), &argv[i], argc - i);
      } else if (ent[0] == '-' && ent[1] != '\n') {
         char *sc = &ent[1];
         while (*sc != '\n') {
         }
      } else {
      }
   }
}

#endif
