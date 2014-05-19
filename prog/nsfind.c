#include <adt.h>
#include <extyp/http.h>

int
main(int argc, char **argv)
{
   // XXX use progargs to ensure an argument is given
   create(inet_addr, ia);
   create_string(hostname, argv[1]);
   ehandle (error, dns_lookup(&hostname, &ia)) {
      if (error_is_type(error, http_addr_hostname_error)) {
         printf("hostname '%s' not found\n", string_to_cstring(&hostname));
         return EXIT_FAILURE;
      } else
         echeck(error);
   }
   inet_addr_print(&ia);
   printf("\n");

   return EXIT_SUCCESS;
}
