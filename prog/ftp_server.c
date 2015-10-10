// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <adt.h>
#include <prog/server_prog.h>
#include <stdtyp/linereader.h>
#include <stdtyp/string_stream.h>

struct ftp_server {
   struct socket control_conn;
   struct socket data_conn;

   bool passive_mode;

   struct string client_address;
   int data_port;
};
adt_func_static(ftp_server);

static void
ftp_server_init(struct ftp_server *self)
{
   socket_init(&self->control_conn);
   socket_init(&self->data_conn);
   self->passive_mode = false;

   string_init(&self->client_address);
   self->data_port = 20;
}

static void
ftp_server_destroy(struct ftp_server *self)
{
   socket_destroy(&self->control_conn);
   socket_destroy(&self->data_conn);
}

static void
ftp_server_set_up(struct ftp_server *self, struct socket **client_conn,
   struct inet_addr *client_addr)
{
   // This is kinda strange
   socket_copy(&self->control_conn, *client_conn);
   (*client_conn)->file.fd = -1;
   socket_free(*client_conn);
   *client_conn = NULL;

   inet_addr_print(client_addr, &self->client_address);
}

static void
_ftp_server_log(struct ftp_server *self, const char *msg, ...)
{
   va_list args;
   va_start(args, msg);

   aprintf("%s (%d): ",
      print(string, &self->client_address), socket_fd(&self->control_conn));
   vprintf(msg, args);
}

#define ftp_server_log(self, ...) adt_print(_ftp_server_log, self, __VA_ARGS__)

a_warn_unused_result
static struct error
ftp_server_send(struct ftp_server *self, int code, const struct string *msg)
{
   create(string, line);
   string_append_format(&line, "%d %s\r\n", code, print(string, msg));
   ereraise(stream_write(socket_to_stream(&self->control_conn), &line));
   ftp_server_log(self, "<-- %s", print(string, &line));

   return no_error;
}

a_warn_unused_result
static struct error
ftp_server_read_command(
   struct ftp_server *self,
   struct string *command_out,
   struct string *value_out)
{
   create_line_reader(client_lr, socket_to_stream(&self->control_conn));

   create(string, line);
   while (string_length(&line) == 0) {
      bool got_line = false;
      ereraise(line_reader_get_line(&client_lr, &line, &got_line));
      if (!got_line) {
         // If we didn't get a line it means the socket it closed, so return an
         // error
         eraise(network_error, "Socket closed.");
      }
   }

   create(string_vec, vec);
   string_split(&line, ' ', &vec);

   string_clear(value_out);
   if (string_vec_size(&vec) >= 1)
      string_copy(command_out, string_vec_at(&vec, 0));
   if (string_vec_size(&vec) == 2)
      string_copy(value_out, string_vec_at(&vec, 1));
   else if(string_vec_size(&vec) > 2)
      panic("command malformed?: %s", print(string, &line));

   ftp_server_log(self, "--> %s\n", print(string, &line));

   return no_error;
}

// Given a string like 127,0,0,1,198,122 convert to 127.0.0.1, 50810
static void
decode_port_string(
   struct string *str,
   struct string *address_out,
   int *port_out)
{
   // Parse out address + port
   create(string_vec, comps);
   string_split(str, ',', &comps);
   adt_assert(string_vec_size(&comps) == 6);

   // Take out the port
   uint8_t first_byte = string_to_int(string_vec_at(&comps, 4));
   uint8_t second_byte = string_to_int(string_vec_at(&comps, 5));
   *port_out = (((uint16_t)first_byte) << 8) | second_byte;
   string_vec_remove_last(&comps);
   string_vec_remove_last(&comps);

   // Take out the address
   string_vec_join(address_out, &comps, '.');
}

// Given an address and port convert to string like 127,0,0,1,198,122
static void
encode_port_string(
   struct inet_addr *iaddr,
   int port,
   struct string *str_out)
{
   string_append_format(str_out, "%u,%u,%u,%u,%d,%d",
      iaddr->addr[0], iaddr->addr[1], iaddr->addr[2], iaddr->addr[3],
      htons(port) & 255, htons(port) >> 8);
}

a_warn_unused_result
static struct error
ftp_server_enter_passive_mode(struct ftp_server *self)
{
   adt_assert(socket_fd(&self->data_conn) == -1);
   self->passive_mode = true;

   ereraise(socket_tcp_init(&self->data_conn));

   // Get the address of the control port. This is the address the client should
   // see us as.
   struct inet_addr iaddr = {};
   ereraise(socket_get_address(&self->control_conn, &iaddr));

   int port = 0;
   ereraise(socket_bind_any_port(&self->data_conn, &port));
   ereraise(socket_listen(&self->data_conn, 1));

   create_string(response, "Entering Passive Mode. ");
   encode_port_string(&iaddr, port, &response);

   ereraise(ftp_server_send(self, 227, &response));

   return no_error;
}

a_warn_unused_result
static struct error
ftp_server_data_connection_on_port(
   struct ftp_server *self,
   struct string *address,
   int port)
{
   adt_assert(socket_fd(&self->data_conn) == -1);
   self->passive_mode = false;

   ftp_server_log(self, "Client updated data address to %s:%d\n",
      print(string, address), port);
   string_copy(&self->client_address, address);
   self->data_port = port;
   ereraise(ftp_server_send(self, 200, strw("PORT successful.")));

   return no_error;
}

static struct error
ftp_server_send_data(struct ftp_server *self, const struct string *data)
{
   // Tell them we are starting data transfer
   ereraise(ftp_server_send(self, 150, strw("Start data transfer.")));

   // Accept a connection on the data port
   if (self->passive_mode) {
      if (socket_fd(&self->data_conn) == -1) {
         ereraise(ftp_server_send(self, 425, strw("Data connection not open")));
         return no_error;
      }
      create(socket, client_conn);
      create(inet_addr, client_addr);
      ereraise(socket_accept(&self->data_conn, &client_conn, &client_addr));
      struct stream *data_stream = socket_to_stream(&client_conn);
      ereraise(stream_write(data_stream, data));
   } else {
      ehandle(error, socket_connect(&self->client_address, self->data_port,
         &self->data_conn)) {
         ereraise(ftp_server_send(self, 425, strw("Data connection not open")));
         return no_error;
      }
      struct stream *data_stream = socket_to_stream(&self->data_conn);
      ereraise(stream_write(data_stream, data));
   }

   // Log the data
   create_string_stream_const(str, data);
   create_line_reader(lr, string_stream_to_stream(&str));
   iter_value (line_reader, &lr, line)
     ftp_server_log(self, "<--data: %s\n", print(string, line));

   // Close the socket
   socket_destroy(&self->data_conn);
   self->passive_mode = false;

   // Tell them data transfer is over
   ereraise(ftp_server_send(self, 226, strw("End data transfer.")));

   return no_error;
}

static struct error
ftp_server_serve_client(struct ftp_server *self)
{
   // Tell the client the connection was established
   ereraise(ftp_server_send(self, 220, strw("FTP server ready.")));

   create(string, command);

   // Read user command
   while (true) {
      create(string, value);
      ehandle(error, ftp_server_read_command(self, &command, &value)) {
         ftp_server_log(self,
            "Closing connection. got %s\n", print(error, &error));
         return no_error;
      }

      if (string_equal(&command, strw("SYST"))) {
         // Ask for system name
         ereraise(ftp_server_send(self, 215, strw("UNIX")));
      } else if (string_equal(&command, strw("USER"))) {
         ereraise(ftp_server_send(self, 331,
            strw("User accepted. Provide password.")));
      } else if (string_equal(&command, strw("PASS"))) {
         ereraise(ftp_server_send(self, 230, strw("User logged in.")));
      } else if (string_equal(&command, strw("PWD"))) {
         // Ask for pwd
         ereraise(ftp_server_send(self, 257,
            strw("\"/\" is the current directory.")));
      } else if (string_equal(&command, strw("PASV"))) {
         ereraise(ftp_server_enter_passive_mode(self));
      } else if (string_equal(&command, strw("PORT"))) {
         create(string, address);
         int port;
         decode_port_string(&value, &address, &port);

         ereraise(ftp_server_data_connection_on_port(self, &address, port));
      } else if (string_equal(&command, strw("LIST"))) {
         // Send some fake listing of files
         ftp_server_send_data(self, strw(
            "total 2\r\n"
            "-r--------    1 user  staff      30 Sep 28  2014 dickbutt\r\n"
            "-r--------    1 user  staff      30 Sep 28  2014 fuck\r\n"
            ));
      } else if (string_equal(&command, strw("RETR"))) {
         // Above both of the files we mentioned we said were 30 bytes, so the
         // data for our fake files also has to be 30 bytes long.
         ftp_server_send_data(self, strw("This is exactly 30 characters!"));
      } else if (string_equal(&command, strw("QUIT"))) {
         ereraise(ftp_server_send(self, 221, strw("Bye.")));
         return no_error;
      } else if (string_equal(&command, strw("TYPE"))) {
         // XXX: Accept any type command
         create(string, resp);
         string_append_format(&resp, "Type set to %s.", print(string, &value));
         ereraise(ftp_server_send(self, 200, &resp));
      } else if (string_equal(&command, strw("CWD"))) {
         // XXX: Sure, we changed directory >.>
         ereraise(ftp_server_send(self, 250, strw("CWD command successful.")));
      } else
         ereraise(ftp_server_send(self, 502, strw("Command not implemented.")));
   }

   return no_error;
}

void
serve_client(struct socket *client_conn, struct inet_addr *client_addr)
{
   aprintf("Got a connection from %s, setting up ftp server\n",
      print(inet_addr, client_addr));

   create(ftp_server, ftp);
   ftp_server_set_up(&ftp, &client_conn, client_addr);

   ehandle(error, ftp_server_serve_client(&ftp)) {
      aprintf("Got an error %s, exiting...\n", print(error, &error));
   }

   inet_addr_free(client_addr);
}

int
main(void)
{
   server_main(21);

   return EXIT_SUCCESS;
}
