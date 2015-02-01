#include <adt.h>
#include <stdtyp/file.h>
#include <extyp/network.h>
#include <stdtyp/linereader.h>
#include <stdtyp/regex.h>

static void
client_404(struct socket *client_conn)
{
   // Send 404 to the given client.  It includes a small body with a header
   // saying 'Not Found'
   const struct string *req =
      strw("HTTP/1.0 404 Not Found\n\n<html><h1>Not Found</h1></html>");
   ecrash(socket_write(client_conn, req));
}

static void
serve_client(struct socket *client_conn, struct inet_addr *client_addr)
{
   // Read off only the first line of the request.
   create(string, buff);
   create_line_reader(client_lr, socket_to_stream(client_conn));
   line_reader_get_line(&client_lr, &buff);

   // Log the request
   aprintf("%s -- \"%s\"\n",
      print(inet_addr, client_addr), print(string, &buff));

   // We only support GET, if it doesn't match the get request, we send a 404
   create_regex(get_reg, strw("GET /([^ ]*) HTTP.*"));
   create(string, path);
   if (!regex_match(&get_reg, &buff, &path)) {
      client_404(client_conn);
      return;
   }

   // Look for the path they requested in the current working directory.  If we
   // couldn't open the path because it doesn't exist, send 404.
   create(file, file);
   ehandle(error, file_open(&file, &path, 0)) {
      if (error_is_type(error, file_not_found_error)) {
         client_404(client_conn);
         return;
      } else {
         ecrash(error);
      }
   }

   // Otherwise we read the file and write the contents back to the client.
   ecrash(file_read(&file, &buff));
   ecrash(socket_write(client_conn, strw("HTTP/1.0 200 OK\n\n")));
   ecrash(socket_write(client_conn, &buff));
}

int
main(void)
{
   // Listen on port 8080 for incoming connections.
   create(socket, tcp_socket);
   ecrash(socket_bind(8080, &tcp_socket));
   // The second argument here is how many connections to queue in the kernel
   // before dropping connections.
   ecrash(socket_listen(&tcp_socket, 10));

   printf("HTTP server started on port 8080\n");

   while (true) {
      // Accept a connection and serve it's request synchronously.
      create(socket, client_conn);
      create(inet_addr, client_addr);
      ecrash(socket_accept(&tcp_socket, &client_conn, &client_addr));
      serve_client(&client_conn, &client_addr);
   }

   return EXIT_SUCCESS;
}
