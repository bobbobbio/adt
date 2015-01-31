#include <adt.h>
#include <stdtyp/file.h>
#include <extyp/network.h>
#include <stdtyp/linereader.h>
#include <stdtyp/regex.h>

static void
client_404(struct file *client_conn)
{
   const struct string *req =
      strw("HTTP/1.0 404 Not Found\n\n<html><h1>Not Found</h1></html>");
   ecrash(stream_write(file_to_stream(client_conn), req));
}

static void
serve_client(struct file *client_conn, struct inet_addr *client_addr)
{
   create(string, buff);
   create_line_reader(client_lr, file_to_stream(client_conn));
   line_reader_get_line(&client_lr, &buff);

   aprintf("%s -- \"%s\"\n",
      print(inet_addr, client_addr), print(string, &buff));

   create_regex(get_reg, strw("GET /([^ ]*) HTTP.*"));
   create(string, path);
   if (!regex_match(&get_reg, &buff, &path)) {
      client_404(client_conn);
      return;
   }


   create(file, file);
   ehandle(error, file_open(&file, &path, 0)) {
      if (error_is_type(error, file_not_found_error)) {
         client_404(client_conn);
         return;
      } else {
         ecrash(error);
      }
   }

   ecrash(stream_read(file_to_stream(&file), &buff));
   ecrash(stream_write(file_to_stream(client_conn),
      strw("HTTP/1.0 200 OK\n\n")));
   ecrash(stream_write(file_to_stream(client_conn), &buff));
}

int
main(void)
{
   create(file, tcp_socket);
   ecrash(tcp_bind(8080, &tcp_socket));
   ecrash(tcp_listen(&tcp_socket, 10));

   printf("HTTP server started on port 8080\n");

   while (true) {
      create(file, client_conn);
      create(inet_addr, client_addr);
      ecrash(tcp_accept(&tcp_socket, &client_conn, &client_addr));
      serve_client(&client_conn, &client_addr);
   }
   printf("exiting??\n");

   return EXIT_SUCCESS;
}
