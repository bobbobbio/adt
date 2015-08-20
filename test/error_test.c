// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <error.h>
#include <stdtyp/file.h>
#include "test.h"

create_error_body(test_error);

struct error b()
{
   eraise(test_error, "Failed to do something b");
}

struct error a()
{
   ereraise(b());

   eraise(test_error, "Failed to do something a");
}

adt_test(error_test)
{
   ehandle(error, a()) {
      // if we get an error, ignore it!
   }

   adt_assert(1 < 2, "this is a message");

   // this one should crash
   b();
}
