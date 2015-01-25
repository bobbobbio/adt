#include <adt.h>
#include <stdtyp/file.h>

int
main(int argc, char **argv)
{
   create(string_vec, files);
   file_list_directory(strw("."), &files);

   adt_assert(string_vec_contains(&files, strw("file_test")));

   return EXIT_SUCCESS;
}
