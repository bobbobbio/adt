// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <extyp/network.h>
#include <extyp/http.h>
#include "test.h"

adt_test(http_test)
{
   create(inet_addr, ia);
   ehandle (error, dns_lookup(strw("127.0.0.1"), &ia)) {
      if (error_is_type(error, network_hostname_error)) {
         panic("network_hostname_error");
      } else
         error_panic(error, "");
   }

   adt_assert(ia.addr[0] == 127
       && ia.addr[1] == 0
       && ia.addr[2] == 0
       && ia.addr[3] == 1);

   create(string, response);
   ecrash(http_get_url(strw("http://abort.cc:80/"), &response));
   adt_assert(string_length(&response) > 0);
   printf("%s", string_to_cstring(&response));
}
