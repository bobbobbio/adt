#ifndef __HTTP_H__
#define __HTTP_H__

#include <adt.h>
#include <stdtyp/string.h>
#include <stdtyp/file.h>

create_error_header(http_addr_error);
create_error_header(http_addr_hostname_error);

struct inet_addr {
   // IPv4 or IPv6
   uint16_t version;
   // Contains a IPv4 (4 bytes) or IPv6 address (16 bytes)
   uint8_t addr[16];
};
adt_func_pod_header(inet_addr);

struct error
dns_lookup(const struct string *dname, struct inet_addr *);

struct error
http_get_url(const struct string *url, struct string *output);


#endif // __HTTP_H__
