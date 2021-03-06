// Copyright 2015 Remi Bernotavicius. All right reserved.

#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <sys/socket.h>

#include <adt.h>
#include <arpa/inet.h>
#include <stdtyp/file.h>
#include <stdtyp/string.h>

create_error_header(network_error);
create_error_header(network_hostname_error);

struct inet_addr {
   // IPv4 or IPv6
   uint16_t version;
   // Contains a IPv4 (4 bytes) or IPv6 address (16 bytes)
   uint8_t addr[16];
};
adt_func_pod_header(inet_addr);

#define inet_addr_ipv4_make(a, b, c, d) \
   ((struct inet_addr){ 4, {a, b, c, d}})

struct socket {
   struct file file;

   int port;
   struct inet_addr address;
};
adt_func_header(socket);

struct error
dns_lookup(const struct string *dname, struct inet_addr *)
   a_warn_unused_result;

int
socket_fd(const struct socket *s);

void
socket_set_fd(struct socket *s, int fd);

struct error
socket_set_bool_option(struct socket *s, int optname);

#define create_socket(name, fd) \
   struct socket name a_cleanup(socket_destroy) = socket_make_var(fd)

struct socket
socket_make_var(int fd);

#define socket_to_stream(sock) \
   file_to_stream(&(sock)->file)

#define socket_read(sock, ...) \
   stream_read(socket_to_stream(sock), __VA_ARGS__)

#define socket_read_n(sock, ...) \
   stream_read_n(socket_to_stream(sock), __VA_ARGS__)

#define socket_read_n_or_less(sock, ...) \
   stream_read_n_or_less(socket_to_stream(sock), __VA_ARGS__)

#define socket_write(sock, ...) \
   stream_write(socket_to_stream(sock), __VA_ARGS__)

#define socket_has_more(sock) \
   stream_has_more(socket_to_stream(sock))

struct error
socket_connect(struct string *server, int port, struct socket *)
   a_warn_unused_result;

struct error
socket_tcp_init(struct socket *socket_out)
   a_warn_unused_result;

struct error
socket_bind(struct socket *tcp_socket, int port)
   a_warn_unused_result;

struct error
socket_bind_any_port(struct socket *tcp_socket, int *port_out)
   a_warn_unused_result;

struct error
socket_listen(struct socket *, int backlog)
   a_warn_unused_result;

struct error
socket_get_port(struct socket *tcp_socket, int *port_out)
   a_warn_unused_result;

struct error
socket_get_address(struct socket *tcp_socket, struct inet_addr *address_out)
   a_warn_unused_result;

struct error
socket_accept(struct socket *, struct socket *socket_out,
   struct inet_addr *client_address)
   a_warn_unused_result;

#endif // __NETWORK_H__
