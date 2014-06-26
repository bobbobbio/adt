#include <adt.h>
#include <extyp/http.h>
#include <stdtyp/argparse.h>

define_args(
   "", '\0', "name", ARG_STRING, ARG_REQUIRED
);

int
arg_main(struct arg_dict *args)
{
   const struct string *hostname = get_arg_string(args, strw(""));
   create(inet_addr, ia);
   ehandle (error, dns_lookup(hostname, &ia)) {
      if (error_is_type(error, http_addr_hostname_error)) {
         printf("hostname '%s' not found\n", string_to_cstring(hostname));
         return EXIT_FAILURE;
      } else
         echeck(error);
   }
   inet_addr_print(&ia);
   printf("\n");

   return EXIT_SUCCESS;
}
