// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <adt.h>
#include <stdtyp/argparse.h>
#include <stdtyp/file.h>
#include <extyp/network.h>
#include <stdtyp/linereader.h>

#include <netdb.h>
#include <sys/socket.h>

define_args(
    "", '\0', "port", ARG_NUM, ARG_REQUIRED
);

int
arg_main(struct arg_dict *args)
{
   int port = get_arg_num(args, strw(""));

   create(socket, tcp_socket);
   ecrash(socket_tcp_init(&tcp_socket));
   ecrash(socket_bind(&tcp_socket, port));
   ecrash(socket_listen(&tcp_socket, 0));

   printf("Listening for connections on port %d\n", port);

   create(socket, client_conn);
   create(inet_addr, client_addr);
   ecrash(socket_accept(&tcp_socket, &client_conn, &client_addr));

   aprintf("Got connection from %s\n\n", print(inet_addr, &client_addr));

   create_line_reader(stdin_lr, file_to_stream(file_stdin));

   create(string, buff);
   while (socket_has_more(&client_conn)) {
      create_file_set(fd_set, &client_conn.file, file_stdin);
      ecrash(file_set_select(&fd_set));

      if (file_set_is_set(&fd_set, &client_conn.file)) {
         ecrash(socket_read_n_or_less(&client_conn, &buff, 1024));
         printf("%s", string_to_cstring(&buff));
      }

      if (file_set_is_set(&fd_set, file_stdin)) {
         ecrash(line_reader_get_line(&stdin_lr, &buff, NULL));
         string_append_cstring(&buff, "\n");
         ecrash(socket_write(&client_conn, &buff));
      }
   }

   return EXIT_SUCCESS;
}
