#include <adt.h>
#include <stdtyp/file.h>

int
main(int argc, char **argv)
{
   create(string_vec, files);
   echeck(file_list_directory(strw("."), &files));

   assert(string_vec_contains(&files, strw("file_test")));

   return EXIT_SUCCESS;
}
