#include <adt.h>
#include <extyp/network.h>
#include <stdtyp/auto_free_pool.h>
#include <stdtyp/file.h>
#include <stdtyp/linereader.h>
#include <stdtyp/regex.h>
#include <stdtyp/signal.h>

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
client_redirect(struct socket *client_conn, const struct string* location)
{
   ecrash(socket_write(client_conn, strw("HTTP/1.0 301 Moved Permanently\n")));
   ecrash(socket_write(client_conn, strw("Location: ")));
   ecrash(socket_write(client_conn, location));
   ecrash(socket_write(client_conn, strw("\n\n")));
}

static void
directory_listing(struct socket *client_conn, struct string_vec *files)
{
   string_vec_sort(files, string_compare);
   ecrash(socket_write(client_conn, strw("HTTP/1.0 200 OK\n\n")));
   ecrash(socket_write(client_conn, strw("<html><body>")));
   iter_value (string_vec, files, path) {
      create(string, line);

      string_append_format(&line, "<a href='%s'>%s</a><br \\>\n",
         print(string, path), print(string, path));

      ecrash(socket_write(client_conn, &line));
   }
   ecrash(socket_write(client_conn, strw("</html></body>")));
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

   if (string_length(&path) == 0)
      string_append_cstring(&path, "./");

   // If they asked for a directory, read the files in the directory and
   // generate a page with links.
   if (path_is_dir(&path)) {
      create(string_vec, files);
      ecrash(file_list_directory(&path, &files));
      if (!string_ends_with(&path, strw("/"))) {
         string_append_char(&path, '/');
         client_redirect(client_conn, &path);
      } else
         directory_listing(client_conn, &files);
   } else {
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
}

struct auto_free_pool *g_pool = NULL;

static void
cleanup_and_exit(void)
{
   if (g_pool != NULL)
      auto_free_pool_free(g_pool);
   exit(1);
}

int main(void)
{
   // Create an auto_free_pool that will get cleaned up on exit.
   g_pool = auto_free_pool_new();
   set_signal_handler(SIGINT, cleanup_and_exit);
   set_signal_handler(SIGTERM, cleanup_and_exit);

   // Make the socket in the pool so that when we exit via signal we always
   // destroy the socket.
   create_ptr_in_pool(g_pool, socket, tcp_socket);
   ecrash(socket_tcp_init(tcp_socket));

   // By setting this option, we can turn around and reuse the socket
   // immediately if you restart.
   ecrash(socket_set_bool_option(tcp_socket, SO_REUSEADDR));

   // Listen on port 8080 for incoming connections.
   ecrash(socket_bind(tcp_socket, 8080));

   // The second argument here is how many connections to queue in the kernel
   // before dropping connections.
   ecrash(socket_listen(tcp_socket, 10));

   printf("HTTP server started on port 8080\n");
   printf("Press Enter to exit...\n");

   create_line_reader(stdin_lr, file_to_stream(file_stdin));

   while (true) {
      create_file_set(fd_set, &tcp_socket->file, file_stdin);
      ecrash(file_set_select(&fd_set));

      if (file_set_is_set(&fd_set, &tcp_socket->file)) {
         // XXX We are potentially leaking the client connection here if we exit
         // on a signal, but it doesn't matter so much, it is an unkown port.
         create(socket, client_conn);
         create(inet_addr, client_addr);

         // Accept a connection and serve it's request synchronously.
         ecrash(socket_accept(tcp_socket, &client_conn, &client_addr));
         serve_client(&client_conn, &client_addr);
      }

      if (file_set_is_set(&fd_set, file_stdin)) {
         printf("Server exiting...\n");
         break;
      }
   }

   auto_free_pool_free(g_pool);

   return EXIT_SUCCESS;
}
