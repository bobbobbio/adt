#include <extyp/http.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>

create_error_body(http_addr_error);

// Error handling
static struct error
eai_to_error(int ecode)
{
   if (ecode == EAI_AGAIN)
      panic("temporary failure in name resolution");
   else if (ecode == EAI_BADFLAGS)
      panic("invalid value for ai_flags");
   else if (ecode == EAI_FAIL)
      return error_make(http_addr_error,
         "non-recoverable failure in name resolution");
   else if (ecode == EAI_FAMILY)
      return error_make(http_addr_error, "ai_family not supported");
   else if (ecode == EAI_MEMORY)
      return error_make(http_addr_error, "memory allocation failure");
   else if (ecode == EAI_NONAME)
      return error_make(http_addr_error,
         "hostname or servname not provided, or not known");
   else if (ecode == EAI_OVERFLOW)
      return error_make(http_addr_error, "argument buffer overflow");
   else if (ecode == EAI_SERVICE)
      return error_make(http_addr_error,
         "servname not supported for ai_socktype");
   else if (ecode == EAI_SOCKTYPE)
      return error_make(http_addr_error, "ai_socktype not supported");
   else if (ecode == EAI_SYSTEM)
      return errno_to_error();
   else
      panic("Invalid error code");

   return no_error;
}

// Taken from the relevant man pages:
// struct addrinfo {
//    int ai_flags;             /* input flags */
//    int ai_family;            /* protocol family for socket */
//    int ai_socktype;          /* socket type */
//    int ai_protocol;          /* protocol for socket */
//    socklen_t ai_addrlen;     /* length of socket-address */
//    struct sockaddr *ai_addr; /* socket-address for socket */
//    char *ai_canonname;       /* canonical name for service location */
//    struct addrinfo *ai_next; /* pointer to next in list */
// };
// Generic sockaddr struct
// struct sockaddr {
//    unsigned short sa_family;
//    char sa_data[14];
// };
// IPv4 specific sockaddr struct
// struct sockaddr_in {
//    short sin_family;
//    unsigned short sin_port
//    struct in_addr sin_addr;
//    char sin_zero[8];
// };
// struct in_addr {
//    uint32_t s_addr;
// };
convert_ctype_static(addrinfo, freeaddrinfo);

adt_func_pod_body(inet_addr);

void
inet_addr_print(const struct inet_addr *a)
{
   assert(a->version == 4);

   printf("%u", a->addr[0]);
   for (unsigned i = 1; i < 4; i++) {
      uint8_t t = a->addr[i];
      printf(".%u", t);
   }
}

struct error
dns_lookup(const struct string *dname, struct inet_addr *iaddr_out)
{
   create(addrinfo, ainfo);
   create_null_ptr(addrinfo, results);

   // AF_UNSPEC is unspecified
   ainfo.ai_family = AF_UNSPEC;
   // SOCK_SRREAM = TCP, SOCK_DGRAM = UDP, DNS is over TCP
   ainfo.ai_socktype = SOCK_STREAM;

   int ecode = 0;

   const char *dname_cstr = string_to_cstring(dname);

   while ((ecode = getaddrinfo(dname_cstr, NULL, &ainfo, &results)) != 0) {
      if (ecode == EAI_AGAIN)
         continue;
      if (ecode == EAI_SYSTEM) {
         if (errno == EINTR || errno == EAGAIN)
            continue;
      }
      return eai_to_error(ecode);
   }

   // TODO: Try using other addresses
   // Check the make sure the address is IPv4
   if (results->ai_addrlen != sizeof(struct sockaddr_in))
      panic("IPv6 is not supported");

   struct sockaddr_in *addr = (struct sockaddr_in *)results->ai_addr;

   iaddr_out->version = 4;
   *((uint32_t *)iaddr_out->addr) = addr->sin_addr.s_addr;

   return no_error;
}

struct error
http_get_url(const struct string *url, struct string *output)
{
   return no_error;
}
