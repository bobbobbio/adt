#include <extyp/http.h>

int
main(int argc, char **argv)
{
   create(inet_addr, ia);
   echeck(dns_lookup(strw("localhost"), &ia));

   assert(ia.addr[0] == 127
       && ia.addr[1] == 0
       && ia.addr[2] == 0
       && ia.addr[3] == 1);

   return EXIT_SUCCESS;
}
