#include <adt.h>
#include <stdtyp/string.h>
#include <stdtyp/subprocess.h>
#include <stdtyp/file.h>

int
main(int argc, char **argv)
{
   create(string_vec, files);
   echeck(file_list_directory(strw("."), &files));

   bool valgrind = false;
   #ifdef __linux
   valgrind = true;
   #endif

   if (valgrind)
      printf("Using valgrind\n");

   iter (string_vec, &files, item) {
      const struct string *file = item.value;
      if (string_ends_with(file, strw("_test"))) {
         printf("%-20s: ", string_to_cstring(file));

         create(string, path);
         if (valgrind) {
            string_append_cstring(&path,
               "valgrind --tool=memcheck --leak-check=full ./");
         } else {
            string_append_cstring(&path, "./");
         }

         string_append_string(&path, file);
         create(string, output);
         struct error e = subprocess_run(&path, &output);
         if (!error_equal(e, no_error)
            || (valgrind && !string_contains_substring(&output,
            strw("All heap blocks were freed -- no leaks are possible"))))
            printf("failed");
         else
            printf("pass");
         printf("\n");
      }
   }

   return EXIT_SUCCESS;
}
