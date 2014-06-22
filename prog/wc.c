#include <adt.h>
#include <stdtyp/argparse.h>

define_args(
    "bytes", 'c', "print the number of bytes", ARG_BOOL, ARG_OPTIONAL,
    "chars", 'm', "print the number of characters", ARG_BOOL, ARG_OPTIONAL,
    "lines", 'l', "print the number of lines", ARG_BOOL, ARG_OPTIONAL,
    "max-line-length", 'L', "print the length of the longest line", ARG_BOOL,
        ARG_OPTIONAL,
    "words", 'w', "print the number of words", ARG_BOOL, ARG_OPTIONAL,
    "version", '\0', "print version information", ARG_BOOL, ARG_OPTIONAL,
    "derp", '\0', "derp?", ARG_STRING, ARG_OPTIONAL,
    "slerp", '\0', "slerp?", ARG_STRING_ARRAY, ARG_OPTIONAL
);

int
arg_main(struct arg_dict *args)
{
   bool *words = get_arg(args, strw("words"));
   if (*words) {
      printf("WORDS!\n");
   }

   const struct string *derp = get_arg(args, strw("derp"));
   if (derp)
      string_print(derp);

   const struct string_vec *slerp = get_arg(args, strw("slerp"));
   if (slerp) {
      string_vec_print(slerp);
   }

   return EXIT_SUCCESS;
}
