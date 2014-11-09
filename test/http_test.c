#include <extyp/http.h>

int
main(int argc, char **argv)
{
   create(inet_addr, ia);
   ehandle (error, dns_lookup(strw("127.0.0.1"), &ia)) {
      if (error_is_type(error, http_addr_hostname_error)) {
         return EXIT_FAILURE;
      } else
         echeck(error);
   }

   assert(ia.addr[0] == 127
       && ia.addr[1] == 0
       && ia.addr[2] == 0
       && ia.addr[3] == 1);

   create(string, response);
   echeck(http_get_url(strw("http://abort.cc:80/"), &response));
   printf("%s", string_to_cstring(&response));

   return EXIT_SUCCESS;
}
