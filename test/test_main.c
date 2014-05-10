#include <adt.h>
#include <stdtyp/string.h>
#include <stdtyp/subprocess.h>
#include <stdtyp/file.h>

int
main(int argc, char **argv)
{
   create(string_vec, files);
   echeck(file_list_directory(strw("."), &files));

   iter (string_vec, &files, item) {
      const struct string *file = item.value;
      if (string_ends_with(file, strw("_test"))) {
         printf("%-20s: ", string_to_cstring(file));

         create_string(path, "valgrind --tool=memcheck --leak-check=full ./");
         string_append_string(&path, file);
         create(string, output);
         struct error e = subprocess_run(&path, &output);
         if (!error_equal(e, no_error) || !string_contains_substring(&output,
               strw("All heap blocks were freed -- no leaks are possible")))
            printf("failed");
         else
            printf("pass");
         printf("\n");
      }
   }

   return EXIT_SUCCESS;
}
