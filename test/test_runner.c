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

   bool a_test_failed = false;
   iter_value (string_vec, &files, file) {
      if (tests != NULL) {
         if (!string_vec_contains(tests, file))
            continue;
      }
      if (string_ends_with(file, strw("_test"))) {
         create_string_format(file_ex, "./%s", print(string, file));
         create_string_vec(list_args, &file_ex, strw("--list"));
         create(string, output);
         ehandle(error, subprocess_run(&list_args, &output)) {
            aprintf("%s\n", print(error, &error));
            aprintf("%-40s: failed to get test list\n", print(string, file));
            a_test_failed = true;
         }
         if (a_test_failed)
            continue;

         create(string_vec, args);
         if (valgrind) {
            string_vec_append_cstrs(&args,
               "valgrind",
               "--tool=memcheck",
               "--leak-check=full",
               "--suppressions=valgrind.supp");
         }
         string_vec_append(&args, &file_ex);

         create(string_vec, tests);
         string_strip(&output);
         string_split(&output, '\n', &tests);
         iter_value (string_vec, &tests, test) {
            create_copy(string, test_name, file);
            string_append_format(&test_name, ":%s", print(string, test));
            printf("%-40s: ", string_to_cstring(&test_name));
            fflush(stdout);

            create_copy(string_vec, path_test, &args);
            string_vec_append(&path_test, test);
            bool failed = false;
            ehandle (error, subprocess_run(&path_test, &output))
               failed = true;
            if (failed) {
               printf("failed\n");
               a_test_failed = true;
               continue;
            }

            if ((valgrind && !string_contains_substring(&output,
               strw("ERROR SUMMARY: 0 errors from 0 contexts")))) {
               printf("failed\n");
               a_test_failed = true;
            } else
               printf("pass\n");
         }
      }
   }

   return a_test_failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
