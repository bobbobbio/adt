// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <adt.h>
#include <prog/server_prog.h>
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

void
serve_client(struct socket *client_conn, struct inet_addr *client_addr)
{
   // Read off only the first line of the request.
   const struct string *line = NULL;
   create_line_reader(client_lr, socket_to_stream(client_conn));
   ecrash(line_reader_get_line(&client_lr, &line));

   // Log the request
   aprintf("%s -- \"%s\"\n",
      print(inet_addr, client_addr), print(string, line));

   // We only support GET, if it doesn't match the get request, we send a 404
   create_regex(get_reg, strw("GET /([^ ]*) HTTP.*"));
   create(string, path);
   if (!regex_match(&get_reg, line, &path)) {
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
      // Look for the path they requested in the current working directory.  If
      // we couldn't open the path because it doesn't exist, send 404.
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
      create(string, buffer);
      ecrash(file_read(&file, &buffer));
      ecrash(socket_write(client_conn, strw("HTTP/1.0 200 OK\n\n")));
      ecrash(socket_write(client_conn, &buffer));
   }
}

int main(void)
{
   server_main(8080);

   return EXIT_SUCCESS;
}
