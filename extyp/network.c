#include <extyp/network.h>
#include <stdtyp/stream.h>

#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

create_error_body(network_error);
create_error_body(network_hostname_error);

adt_func_body(socket);

void
socket_init(struct socket *s)
{
   file_init(&s->file);
}

void
socket_destroy(struct socket *s)
{
   ecrash(file_close(&s->file));
   file_destroy(&s->file);
}

static struct error
eai_to_error(int ecode)
{
   if (ecode == EAI_AGAIN)
      panic("temporary failure in name resolution");
   else if (ecode == EAI_BADFLAGS)
      panic("invalid value for ai_flags");
   else if (ecode == EAI_FAIL)
      eraise(network_error,
         "non-recoverable failure in name resolution");
   else if (ecode == EAI_FAMILY)
      eraise(network_error, "ai_family not supported");
   else if (ecode == EAI_MEMORY)
      eraise(network_error, "memory allocation failure");
   else if (ecode == EAI_NONAME)
      eraise(network_hostname_error,
         "hostname or servname not provided, or not known");
   else if (ecode == EAI_OVERFLOW)
      eraise(network_error, "argument buffer overflow");
   else if (ecode == EAI_SERVICE)
      eraise(network_error,
         "servname not supported for ai_socktype");
   else if (ecode == EAI_SOCKTYPE)
      eraise(network_error, "ai_socktype not supported");
   else if (ecode == EAI_SYSTEM)
      ereraise(errno_to_error());
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
   adt_assert(a->version == 4);

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
   // Check to make sure the address is IPv4
   if (results->ai_addrlen != sizeof(struct sockaddr_in))
      panic("IPv6 is not supported");

   struct sockaddr_in *addr = (struct sockaddr_in *)results->ai_addr;

   iaddr_out->version = 4;
   *((uint32_t *)iaddr_out->addr) = addr->sin_addr.s_addr;

   return no_error;
}

int
socket_fd(struct socket *s)
{
   return file_fd(&s->file);
}

struct socket
socket_make_var(int fd)
{
   struct socket s;
   socket_init(&s);
   s.file.fd = fd;

   return s;
}

void
socket_copy(struct socket *d, const struct socket *s)
{
   file_copy(&d->file, &s->file);
}

struct error
socket_connect(struct string *server, int port, struct socket* socket_out)
{
   // Do the DNS lookup
   create(inet_addr, ia);
   ereraise(dns_lookup(server, &ia));

   // Create tcp connection
   create_socket(tcp_socket, socket(AF_INET, SOCK_STREAM, 0));

   if (socket_fd(&tcp_socket) == -1)
      return errno_to_error();

   struct sockaddr_in saddr = {
      .sin_family = AF_INET,
      .sin_port = htons(port),
      .sin_addr.s_addr = *((uint32_t *)ia.addr)
   };

   // Connect to the server
   if (connect(socket_fd(&tcp_socket),
      (struct sockaddr *)&saddr, sizeof(saddr)) != 0)
      return errno_to_error();

   socket_copy(socket_out, &tcp_socket);
   tcp_socket.file.fd = -1;

   return no_error;
}

struct error
socket_bind(int port, struct socket *socket_out)
{
   create_socket(tcp_socket, socket(AF_INET, SOCK_STREAM, 0));

   if (socket_fd(&tcp_socket) == -1)
      ereraise(errno_to_error());

   struct sockaddr_in saddr = {
      .sin_family = AF_INET,
      .sin_port = htons(port),
      .sin_addr.s_addr = INADDR_ANY
   };

   int err =
      bind(socket_fd(&tcp_socket), (struct sockaddr *)&saddr, sizeof(saddr));

   if (err != 0)
      ereraise(errno_to_error());

   socket_copy(socket_out, &tcp_socket);
   tcp_socket.file.fd = -1;

   return no_error;
}

struct error
socket_listen(struct socket *tcp_socket, int backlog)
{
   int err = listen(socket_fd(tcp_socket), backlog);

   if (err != 0)
      ereraise(errno_to_error());

   return no_error;
}

struct error
socket_accept(struct socket *tcp_socket, struct socket *socket_out,
   struct inet_addr *client_address)
{
   struct sockaddr_in caddr;
   socklen_t clen = sizeof(caddr);
   create_socket(client_conn,
      accept(socket_fd(tcp_socket), (struct sockaddr *)&caddr, &clen));

   if (socket_fd(&client_conn) == -1)
      ereraise(errno_to_error());

   create(inet_addr, client_addr);
   client_addr.version = 4;
   *((uint32_t *)client_addr.addr) = caddr.sin_addr.s_addr;

   socket_copy(socket_out, &client_conn);
   client_conn.file.fd = -1;

   if (client_address != NULL)
      inet_addr_copy(client_address, &client_addr);

   return no_error;
}
