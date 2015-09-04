// Copyright 2015 Remi Bernotavicius. All right reserved.

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
   ecrash(file_list_directory(strw("."), &files));
   string_vec_sort(&files, string_compare);

   bool valgrind = true;
   #ifdef __APPLE__
   // Valgrind is broken on OS X
   valgrind = false;
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
         create(string, path);
         if (valgrind) {
            string_append_cstring(&path,
               "valgrind --tool=memcheck --leak-check=full "
               "--suppressions=valgrind.supp ./");
         } else {
            string_append_cstring(&path, "./");
         }
         string_append_string(&path, file);

         create_copy(string, path_list, &path);
         string_append_cstring(&path_list, " --list");
         create(string, output);
         ehandle(error, subprocess_run(&path_list, &output)) {
            printf("%-40s: failed to get test list\n", string_to_cstring(file));
            continue;
         }
         create(string_vec, tests);
         string_split(&output, '\n', &tests);
         iter_value (string_vec, &tests, test) {
            create_copy(string, test_name, file);
            string_append_format(&test_name, ":%s", print(string, test));
            printf("%-40s: ", string_to_cstring(&test_name));
            fflush(stdout);

            create_copy(string, path_test, &path);
            string_append_format(&path_test, " %s", print(string, test));
            bool failed = false;
            ehandle (error, subprocess_run(&path_test, &output))
               failed = true;
            if (failed) {
               printf("failed\n");
               continue;
            }

            if ((valgrind && !string_contains_substring(&output,
               strw("ERROR SUMMARY: 0 errors from 0 contexts"))))
               printf("failed\n");
            else
               printf("pass\n");
         }
      }
   }

   return EXIT_SUCCESS;
}
