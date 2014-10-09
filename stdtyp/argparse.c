#include <stdtyp/argparse.h>

adt_func_body(arg_dict);
iter_gen_body(arg_dict, struct string *, void *);

map_gen_body(string_arg_template_map, string, arg_template);
map_gen_body(string_arg_map, string, arg_value);
map_gen_podk_body(char_string_map, char, string);

struct arg_template {
   struct string description;
   enum arg_needs_type needs_type;
   enum arg_type type;
   char sc;
};
adt_func_body(arg_template);

struct arg_value {
   enum arg_type type;
   void *value;
};
adt_func_pod_body(arg_value);

static void
arg_value_free_value(struct arg_value *av)
{
   switch (av->type) {
      case ARG_STRING:
         string_free(av->value);
      break;
      case ARG_STRING_ARRAY:
         string_vec_free(av->value);
      break;
      case ARG_BOOL:
      case ARG_NUM:
         free(av->value);
      break;
   }
}

static void *
get_arg_no_type(const struct arg_dict *dict, const struct string *cmd)
{
   // Make sure this is a real argument
   assert(string_arg_template_map_contains(&dict->templates, cmd));

   if (!string_arg_map_contains(&dict->values, cmd)) {
      // If we don't have the arg, it had to have been an optional one
      assert(string_arg_template_map_at(&dict->templates, cmd)->needs_type ==
         ARG_OPTIONAL);
      return NULL;
   }

   return string_arg_map_at(&dict->values, cmd)->value;
}

void *
get_arg(const struct arg_dict *dict, enum arg_type type,
   const struct string *cmd)
{
   void *arg = get_arg_no_type(dict, cmd);
   assert(string_arg_template_map_at(&dict->templates, cmd)->type == type);

   return arg;
}

enum arg_type
get_arg_type(const struct arg_dict *dict, const struct string *cmd)
{
   // Make sure this is a real argument
   assert(string_arg_template_map_contains(&dict->templates, cmd));

   return string_arg_template_map_at(&dict->templates, cmd)->type;
}

bool
has_arg(const struct arg_dict *dict, const struct string *cmd)
{
   void *a = get_arg_no_type(dict, cmd);
   if (string_arg_template_map_at(&dict->templates, cmd)->type == ARG_BOOL) {
      assert(a != NULL);
      return *(bool *)a;
   }

   return a != NULL;
}

bool
get_arg_bool(const struct arg_dict *dict, const struct string *cmd)
{
   return *(bool *)get_arg(dict, ARG_BOOL, cmd);
}

const struct string *
get_arg_string(const struct arg_dict *dict, const struct string *cmd)
{
   return get_arg(dict, ARG_STRING, cmd);
}

int
get_arg_num(const struct arg_dict *dict, const struct string *cmd)
{
   return *(int *)get_arg(dict, ARG_NUM, cmd);
}

const struct string_vec *
get_arg_string_array(const struct arg_dict *dict, const struct string *cmd)
{
   return get_arg(dict, ARG_STRING_ARRAY, cmd);
}

bool
arg_dict_iter_next(const struct arg_dict *ad, struct arg_dict_iter *iter)
{
   create(string_arg_template_map_iter, inner_iter);
   inner_iter.pos = iter->pos;
   bool cont = string_arg_template_map_iter_next(&ad->templates, &inner_iter);

   if (cont) {
      iter->pos = inner_iter.pos;
      iter->key = inner_iter.key;
      iter->value = get_arg_no_type(ad, iter->key);

      return true;
   } else
      return false;
}

void
_declare_args(struct arg_dict *ad, const char *arg_name, ...)
{
   va_list argp;

   va_start(argp, arg_name);

   while (arg_name != NULL) {
      create(arg_template, at);

      char sc = (char)va_arg(argp, unsigned);
      const char *desc = va_arg(argp, const char *);
      string_append_cstring(&at.description, desc);
      at.type = va_arg(argp, enum arg_type);
      at.needs_type = va_arg(argp, enum arg_needs_type);
      at.sc = sc;

      create_string(arg_name_str, arg_name);
      string_arg_template_map_insert(&ad->templates, &arg_name_str, &at);

      if (sc != '\0')
         char_string_map_insert(&ad->shortcuts, sc, &arg_name_str);

      arg_name = va_arg(argp, const char *);
   }

   va_end(argp);
}

void
arg_template_init(struct arg_template *a)
{
   string_init(&a->description);
   a->needs_type = ARG_OPTIONAL;
   a->type = ARG_BOOL;
   a->sc = '\0';
}

void
arg_template_destroy(struct arg_template *a)
{
   string_destroy(&a->description);
}

void
arg_template_copy(struct arg_template *dst, const struct arg_template *src)
{
   dst->needs_type = src->needs_type;
   dst->type = src->type;
   dst->sc = src->sc;
   string_copy(&dst->description, &src->description);
}

void
arg_dict_init(struct arg_dict *d)
{
   string_arg_template_map_init(&d->templates);
   char_string_map_init(&d->shortcuts);
   string_arg_map_init(&d->values);
   d->prog_name = NULL;
}

void
arg_dict_destroy(struct arg_dict *d)
{
   string_arg_template_map_destroy(&d->templates);
   char_string_map_destroy(&d->shortcuts);

   iter (string_arg_map, &d->values, item)
      arg_value_free_value(item.value);

   string_arg_map_destroy(&d->values);
}

void
arg_dict_copy(struct arg_dict *dst, const struct arg_dict *src)
{
   string_arg_template_map_copy(&dst->templates, &src->templates);
   char_string_map_copy(&dst->shortcuts, &src->shortcuts);
   string_arg_map_copy(&dst->values, &src->values);
}

static void
arg_print_help(struct arg_dict *dict)
{
   assert(dict->prog_name != NULL);

   bool multi_options;
   if (string_arg_template_map_contains(&dict->templates, strw("")))
      multi_options = string_arg_template_map_size(&dict->templates) > 2;
   else
      multi_options = string_arg_template_map_size(&dict->templates) > 1;

   printf("Usage: %s [OPTION]%s", dict->prog_name,
      multi_options ? "..." : "");

   if (string_arg_template_map_contains(&dict->templates, strw(""))) {
      struct arg_template *at =
         string_arg_template_map_at(&dict->templates, strw(""));
      printf(" [%s]%s", string_to_cstring(&at->description),
         at->type == ARG_STRING_ARRAY ? "..." : "");
   }

   printf("\n");
   printf("Options:\n");
   iter (string_arg_template_map, &dict->templates, item) {
      if (string_equal(item.key, strw("")))
         continue;
      if (item.value->sc != '\0')
         printf("-%c, ", item.value->sc);
      else
         printf("    ");
      printf("--");
      printf("%s", string_to_cstring(item.key));
      printf("\t");
      printf("%s", string_to_cstring(&item.value->description));
      printf("\n");
   }

   printf("    --help\tprint help and exit\n");

   exit(EXIT_FAILURE);
}

static int
process_arg(struct arg_dict *dict, const struct string *arg_name,
   struct arg_value *av, char **argv, int len)
{
   int num_used = 0;
   int i = 0;
   for ( ; i < len && !(argv[i][0] == '-' && argv[i][1] != '\0'); i++) {
      switch (av->type) {
         case ARG_STRING: {
            if (av->value != NULL) {
               afprintf(stderr, "too many values given to argument '%s'\n",
                  print(string, arg_name));
               arg_print_help(dict);
            }  else {
               av->value = (void *)string_const_new(argv[i]);
            }
            num_used++;
         }
         break;
         case ARG_NUM: {
            if (av->value != NULL) {
               afprintf(stderr, "too many values given to argument '%s'\n",
                  print(string, arg_name));
               arg_print_help(dict);
            } else {
               av->value = malloc(sizeof(int));
               *(int *)av->value = atoi(argv[i]);
            }
            num_used++;
         }
         break;
         case ARG_STRING_ARRAY: {
            if (av->value == NULL)
               av->value = string_vec_new();
            create_string(arg_str, argv[i]);
            string_vec_append(av->value, &arg_str);
            num_used++;
         }
         break;
         case ARG_BOOL:
         break;
      }
   }
   if (av->type == ARG_BOOL) {
      av->value = malloc(sizeof(bool));
      *(bool *)av->value = true;
   } else {
      if (av->value == NULL) {
         fprintf(stderr, "argument '%s' requres value\n",
            string_to_cstring(arg_name));
         arg_print_help(dict);
      }
   }

   return num_used;
}

void
parse_args(struct arg_dict *dict, char **argv, int argc)
{
   dict->prog_name = argv[0];

   for (int i = 1; i < argc; i++) {
      char *arg = argv[i];
      if (arg[0] == '-' && arg[1] == '-') {
         create_string(command, &arg[2]);
         if (string_equal(&command, strw("help")))
            arg_print_help(dict);
         if (string_arg_template_map_contains(&dict->templates, &command)) {
            struct arg_template *at =
               string_arg_template_map_at(&dict->templates, &command);
            if (string_arg_map_contains(&dict->values, &command)) {
               fprintf(stderr, "command '%s' provided twice\n",
                  string_to_cstring(&command));
               arg_print_help(dict);
            }
            create(arg_value, av);
            av.type = at->type;
            i += process_arg(dict, &command, &av, &argv[i + 1], argc - i - 1);
            string_arg_map_insert(&dict->values, &command, &av);
         } else {
            fprintf(stderr, "unknown command '%s'\n", arg);
            arg_print_help(dict);
         }
      } else if (arg[0] == '-' && arg[1] != '\0') {
         for (char *c = &arg[1]; *c != '\0'; c++) {
            if (char_string_map_contains(&dict->shortcuts, *c)) {
               create(string, command);
               char_string_map_get(&dict->shortcuts, *c, &command);
               struct arg_template *at =
                  string_arg_template_map_at(&dict->templates, &command);
               if (string_arg_map_contains(&dict->values, &command)) {
                  fprintf(stderr, "command '%s' provided twice\n",
                     string_to_cstring(&command));
                  arg_print_help(dict);
               }
               create(arg_value, av);
               av.type = at->type;
               i += process_arg(dict, &command, &av, &argv[i + 1],
                  argc - i - 1);
               string_arg_map_insert(&dict->values, &command, &av);
            } else {
               fprintf(stderr, "%s: invalid option '%c'\n", argv[0], *c);
               arg_print_help(dict);
            }
         }
      }  else {
         if (string_arg_template_map_contains(&dict->templates, strw(""))) {
            struct arg_template *at =
               string_arg_template_map_at(&dict->templates, strw(""));
            if (string_arg_map_contains(&dict->values, strw(""))) {
               fprintf(stderr, "unexpected argument: '%s'", arg);
               arg_print_help(dict);
            }
            create(arg_value, av);
            av.type = at->type;
            i += process_arg(dict, strw(""), &av, &argv[i], argc - i) - 1;
            string_arg_map_insert(&dict->values, strw(""), &av);
         } else {
            fprintf(stderr, "unexpected argument: '%s'", arg);
            arg_print_help(dict);
         }
      }
   }

   iter (string_arg_template_map, &dict->templates, item) {
      if (!string_arg_map_contains(&dict->values, item.key)) {
         if (item.value->needs_type == ARG_REQUIRED) {
            fprintf(stderr, "required arg '%s' missing\n",
               string_to_cstring(item.key));
            arg_print_help(dict);
         }
         if (item.value->type == ARG_BOOL) {
            create(arg_value, av);
            av.type = ARG_BOOL;
            av.value = malloc(sizeof(bool));
            *(bool *)av.value = false;
            string_arg_map_insert(&dict->values, item.key, &av);
         }
      }
   }
}


