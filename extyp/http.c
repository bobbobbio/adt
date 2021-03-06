// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <extyp/http.h>
#include <stdtyp/linereader.h>
#include <stdtyp/map.h>
#include <stdtyp/regex.h>
#include <extyp/network.h>

map_gen_static(string_string_map, string, string);

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
   create(socket, tcp_stream);
   ereraise(socket_connect(&domain, port, &tcp_stream));

   // Send the GET request
   create(string, req);
   // Use HTTP/1.0 because otherwise the server will try to keep the connection
   // alive, and we don't support that exactly.
   string_append_format(&req, "GET %s HTTP/1.0\n", print(string, &path));
   string_append_format(&req, "host: %s\n\n", print(string, &domain));
   ereraise(socket_write(&tcp_stream, &req));

   create_line_reader(lr, socket_to_stream(&tcp_stream));

   create_regex(kv_reg, strw("(.+): (.+)"));
   create(string_string_map, header);
   int line_number = 0;
   while (true) {
      const struct string *line = NULL;
      ecrash(line_reader_get_line(&lr, &line));
      // status line
      if (line_number == 0) {
         // XXX Check for HTTP OK or something
      } else {
         create_copy(string, line_stripped, line);
         string_strip(&line_stripped);

         // Check for end of header
         if (string_length(&line_stripped) == 0)
            break;
         create(string, key);
         create(string, value);
         // XXX Rather than assert return some error
         adt_assert(regex_match(&kv_reg, &line_stripped, &key, &value));
         string_string_map_insert(&header, &key, &value);
      }
      line_number++;
   }

   // Read the whole body, now that we got all the headers.
   iter_value (line_reader, &lr, line)
      string_append_format(output, "%s\n", string_to_cstring(line));

   return no_error;
}
