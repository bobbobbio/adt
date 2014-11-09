#include <stdtyp/map.h>
#include <stdtyp/linereader.h>
#include <extyp/http.h>
#include <stdtyp/regex.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <limits.h>

create_error_body(http_addr_error);
create_error_body(http_addr_hostname_error);

map_gen_static(string_string_map, string, string);

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
      return error_make(http_addr_hostname_error,
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
inet_addr_print(const struct inet_addr *a, struct string *s)
{
   assert(a->version == 4);

   string_append_format(s, "%u", a->addr[0]);
   for (unsigned i = 1; i < 4; i++) {
      uint8_t t = a->addr[i];
      string_append_format(s, ".%u", t);
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
tcp_connect(struct string *server, int port, int *fd_out)
{
   // Do the DNS lookup
   create(inet_addr, ia);
   epass(dns_lookup(server, &ia));

   // Create tcp connection
   create_fd(fd, socket(AF_INET, SOCK_STREAM, 0));
   if (fd == -1)
      return errno_to_error();

   struct sockaddr_in saddr = {
      .sin_family = AF_INET,
      .sin_port = htons(port),
      .sin_addr.s_addr = *((uint32_t *)ia.addr)
   };

   // Connect to the server
   if (connect(fd, (struct sockaddr *)&saddr, sizeof(saddr)) != 0)
      return errno_to_error();

   *fd_out = fd;
   fd = -1;

   return no_error;
}

struct error
http_get_url(const struct string *url, struct string *output)
{
   string_clear(output);

   // Parse the url
   create_regex(url_reg, strw("(.+):\\/\\/([^\\/]+)(\\/.*)"));

   create(string, protocol);
   create(string, domain);
   create(string, path);
   // XXX Lets return an error and not panic
   if (!regex_match(&url_reg, url, &protocol, &domain, &path))
      panic("Not well formed url: '%s'", print(string, url));

   // Parse out the port
   int port = 80;
   if (string_contains_char(&domain, ':')) {
      create_regex(port_reg, strw("(.+):([0-9]+)"));
      create(string, ndomain);
      create(string, sport);
      if (!regex_match(&port_reg, &domain, &ndomain, &sport))
         panic("Domain not well formed: '%s'", print(string, &domain));
      string_copy(&domain, &ndomain);
      port = string_to_int(&sport);
   }

   // Create the tcp connection
   create_fd(fd, -1);
   epass(tcp_connect(&domain, port, &fd));

   // Send the GET request
   create(string, req);
   string_append_format(&req, "GET %s HTTP/1.1\n", print(string, &path));
   string_append_format(&req, "host: %s\n\n", print(string, &domain));
   epass(string_write_fd(&req, fd));

   create(line_reader, lr);
   line_reader_open_fd(&lr, fd);

   create_regex(kv_reg, strw("(.+): (.+)"));
   create(string_string_map, header);
   int line_number = 0;
   while (true) {
      create(string, line);
      line_reader_get_line(&lr, &line);
      // status line
      if (line_number == 0) {
         // XXX Check for HTTP OK or something
      } else {
         // Check for end of header
         if (string_length(&line) == 0)
            break;
         create(string, key);
         create(string, value);
         // XXX Rather than assert return some error
         assert(regex_match(&kv_reg, &line, &key, &value));
         string_string_map_insert(&header, &key, &value);
      }
      line_number++;
   }

   // XXX fix this!
   if (string_string_map_contains(&header, strw("Transfer-Encoding"))) {
      if (string_equal(string_string_map_at(&header, strw("Transfer-Encoding")),
         strw("chunked")))
         panic("chunked encoding not supported");
   }

   // Start of content
   int clength = INT_MAX;
   if (string_string_map_contains(&header, strw("Content-Length")))
      clength = string_to_int(string_string_map_at(&header,
         strw("Content-Length")));

   create(string, line);
   while (line_reader_get_line(&lr, &line)) {
      string_append_format(output, "%s\n", string_to_cstring(&line));
      // XXX This isn't exact, because we're eating \r doh!
      clength -= (string_length(&line) + 1);
      if (clength <= 0)
         break;
   }

   return no_error;
}
