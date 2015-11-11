// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <extyp/network.h>
#include <stdtyp/stream.h>

#include <errno.h>
#include <limits.h>
#include <netdb.h>
#include <sys/types.h>

create_error_body(network_error);
create_error_body(network_hostname_error);

adt_func_body(socket);

void
socket_init(struct socket *s)
{
   file_init(&s->file);

   s->port = -1;
   inet_addr_init(&s->address);
}

void
socket_destroy(struct socket *s)
{
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
      eraise_errno_error();
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
   memcpy(iaddr_out->addr, &addr->sin_addr.s_addr, 4);

   return no_error;
}

int
socket_fd(const struct socket *s)
{
   return file_fd(&s->file);
}

void
socket_set_fd(struct socket *s, int fd)
{
   file_set_fd(&s->file, fd);
}

void
socket_print(const struct socket *s, struct string *str)
{
   string_append_format(str, "socket(fd=%d)", socket_fd(s));
}

struct socket
socket_make_var(int fd)
{
   struct socket s;
   socket_init(&s);
   s.file.fd = fd;

   return s;
}

struct error
socket_set_bool_option(struct socket *s, int optname)
{
   int val = 1;
   int error = setsockopt(socket_fd(s), SOL_SOCKET, optname, &val, sizeof(val));
   if (error != 0)
      eraise_errno_error();

   return no_error;
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
   create(socket, tcp_socket);

   do {
      socket_set_fd(&tcp_socket, socket(AF_INET, SOCK_STREAM, 0));

      if (socket_fd(&tcp_socket) == -1) {
         if (errno != EAGAIN && errno != EINTR)
            eraise_errno_error();
      } else
         break;
   } while (true);

   struct sockaddr_in saddr = {
      .sin_family = AF_INET,
      .sin_port = htons(port),
   };
   memcpy(&saddr.sin_addr.s_addr, &ia.addr, 4);

   // Connect to the server
   do {
      int error = connect(
         socket_fd(&tcp_socket), (struct sockaddr *)&saddr, sizeof(saddr));
      if (error != 0) {
         if (errno != EAGAIN && errno != EINTR)
            eraise_errno_error();
      } else
         break;
   } while (true);

   socket_copy(socket_out, &tcp_socket);
   socket_set_fd(&tcp_socket, -1);

   return no_error;
}

struct error
socket_tcp_init(struct socket *socket_out)
{
   create(socket, tcp_socket);

   do {
      socket_set_fd(&tcp_socket, socket(AF_INET, SOCK_STREAM, 0));

      if (socket_fd(&tcp_socket) == -1) {
         if (errno != EAGAIN && errno != EINTR)
            eraise_errno_error();
      } else
         break;
   } while (true);

   socket_copy(socket_out, &tcp_socket);
   tcp_socket.file.fd = -1;

   return no_error;
}

struct error
_socket_bind(struct socket *tcp_socket, int port)
{
   struct sockaddr_in saddr = {
      .sin_family = AF_INET,
      .sin_port = htons(port),
      .sin_addr.s_addr = INADDR_ANY
   };

   do {
      int err =
         bind(socket_fd(tcp_socket), (struct sockaddr *)&saddr, sizeof(saddr));

      if (err != 0) {
         if (errno != EAGAIN && errno != EINTR)
            eraise_errno_error();
      } else
         break;
   } while (true);

   return no_error;
}

struct error
socket_bind(struct socket *tcp_socket, int port)
{
   ereraise(_socket_bind(tcp_socket, port));

   return no_error;
}

static struct error
socket_getsockname(struct socket *tcp_socket)
{
   struct sockaddr_in saddr = {};
   socklen_t len = sizeof(saddr);

   int err = getsockname(
      socket_fd(tcp_socket), (struct sockaddr *)&saddr, &len);

   if (err == -1)
      eraise_errno_error();

   tcp_socket->port = ntohs(saddr.sin_port);

   tcp_socket->address.version = 4;
   memcpy(tcp_socket->address.addr, &saddr.sin_addr.s_addr, 4);

   return no_error;
}

struct error
socket_get_port(struct socket *tcp_socket, int *port_out)
{
   if (tcp_socket->port == -1)
      ereraise(socket_getsockname(tcp_socket));

   *port_out = tcp_socket->port;

   return no_error;
}

struct error
socket_get_address(struct socket *tcp_socket, struct inet_addr *address_out)
{
   if (tcp_socket->address.version == 0)
      ereraise(socket_getsockname(tcp_socket));

   inet_addr_copy(address_out, &tcp_socket->address);

   return no_error;
}

struct error
socket_bind_any_port(struct socket *tcp_socket, int *port_out)
{
   ereraise(_socket_bind(tcp_socket, INADDR_ANY));

   if (port_out != NULL)
      ereraise(socket_get_port(tcp_socket, port_out));

   return no_error;
}

struct error
socket_listen(struct socket *tcp_socket, int backlog)
{
   do {
      int err = listen(socket_fd(tcp_socket), backlog);

      if (err != 0) {
         if (errno != EAGAIN && errno != EINTR)
            eraise_errno_error();
      } else
         break;
   } while (true);

   return no_error;
}

struct error
socket_accept(struct socket *tcp_socket, struct socket *socket_out,
   struct inet_addr *client_address)
{
   struct sockaddr_in caddr;
   socklen_t clen = sizeof(caddr);
   create(socket, client_conn);

   do {
      socket_set_fd(&client_conn,
         accept(socket_fd(tcp_socket), (struct sockaddr *)&caddr, &clen));

      if (socket_fd(&client_conn) == -1) {
         if (errno != EAGAIN && errno != EINTR)
            eraise_errno_error();
      } else
         break;
   } while (true);

   create(inet_addr, client_addr);
   client_addr.version = 4;
   memcpy(client_addr.addr, &caddr.sin_addr.s_addr, 4);

   socket_copy(socket_out, &client_conn);
   client_conn.file.fd = -1;

   if (client_address != NULL)
      inet_addr_copy(client_address, &client_addr);

   return no_error;
}
