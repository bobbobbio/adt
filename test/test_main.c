#include <adt.h>
#include <stdtyp/string.h>
#include <stdtyp/subprocess.h>
#include <stdtyp/file.h>
#include <stdtyp/argparse.h>

define_args(
   "", '\0', "tests to run", ARG_STRING_ARRAY, ARG_OPTIONAL
);

int
arg_main(struct arg_dict *args)
{
   create(string_vec, files);
   epanic(file_list_directory(strw("."), &files));

   bool valgrind = false;
   #ifdef __linux
   valgrind = true;
   #endif

   if (valgrind)
      printf("Using valgrind\n");

   const struct string_vec *tests = get_arg_string_array(args, strw(""));

   iter_value (string_vec, &files, file) {
      if (tests != NULL) {
         if (!string_vec_contains(tests, file))
            continue;
      }
      if (string_ends_with(file, strw("_test"))) {
         printf("%-20s: ", string_to_cstring(file));

         create(string, path);
         if (valgrind) {
            string_append_cstring(&path,
               "valgrind --tool=memcheck --leak-check=full "
               "--suppressions=valgrind.supp ./");
         } else {
            string_append_cstring(&path, "./");
         }

         string_append_string(&path, file);
         create(string, output);
         struct error e = subprocess_run(&path, &output);
         if (!error_equal(e, no_error)
            || (valgrind && !string_contains_substring(&output,
            strw("ERROR SUMMARY: 0 errors from 0 contexts"))))
            printf("failed");
         else
            printf("pass");
         printf("\n");
      }
   }

   return EXIT_SUCCESS;
}
