#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <adt.h>
#include <stdtyp/string.h>
#include <stdtyp/file.h>

create_error_header(network_error);
create_error_header(network_hostname_error);

struct inet_addr {
   // IPv4 or IPv6
   uint16_t version;
   // Contains a IPv4 (4 bytes) or IPv6 address (16 bytes)
   uint8_t addr[16];
};
adt_func_pod_header(inet_addr);

struct error
dns_lookup(const struct string *dname, struct inet_addr *)
   a_warn_unused_result;

struct error
tcp_connect(struct string *server, int port, struct file *)
   a_warn_unused_result;

struct error
tcp_bind(int port, struct file *stream_out)
   a_warn_unused_result;

struct error
tcp_listen(struct file *tcp_socket, int backlog)
   a_warn_unused_result;

struct error
tcp_accept(struct file *tcp_socket, struct file *file_out,
   struct inet_addr *client_address)
   a_warn_unused_result;

#endif // __NETWORK_H__
