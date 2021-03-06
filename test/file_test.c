// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <adt.h>
#include <stdtyp/file.h>
#include "test.h"

adt_test(file_test)
{
   create(string_vec, files);
   ecrash(file_list_directory(strw("/etc"), &files));

   adt_assert(path_is_dir(strw(".")));
   adt_assert(!path_is_dir(strw("/etc/hosts")));

   adt_assert(string_vec_contains(&files, strw("hosts")));
}
