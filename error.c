#include <error.h>

create_error_body(_no_error);

void
error_panic(struct error e, char *code)
{
   fprintf(stderr, "Got error: %s : \"%s\" : %s\n",
      e.type, code, e.msg);
   abort();
}

void
panic(char *msg)
{
   fprintf(stderr, "%s\n", msg);
   abort();
}

char *
error_msg(struct error e)
{
   return e.msg;
}

