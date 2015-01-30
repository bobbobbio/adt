#include <adt.h>
#include <extyp/network.h>
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
      if (error_is_type(error, network_hostname_error)) {
         aprintf("hostname '%s' not found\n", print(string, hostname));
         return EXIT_FAILURE;
      } else
         error_panic(error, "Failed to look up hostname");
   }
   aprintf("%s\n", print(inet_addr, &ia));

   return EXIT_SUCCESS;
}
