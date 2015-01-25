#include <error.h>
#include <stdtyp/file.h>

create_error_body(test_error);

struct error b()
{
   raise(test_error, "Failed to do something b");
}

struct error a()
{
   reraise(b());

   raise(test_error, "Failed to do something a");
}

int main()
{
   ehandle(error, a()) {
      // if we get an error, ignore it!
   }

   adt_assert(1 < 2, "this is a message");

   // this one should crash
   b();
}
