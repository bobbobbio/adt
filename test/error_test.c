#include <error.h>

create_error(test_error);

struct error a()
{
   return error_make(test_error, "Failed to do something");
}

struct error b()
{
   return no_error;
}

int main()
{
   echeck(b());

   echeck(a());
}
