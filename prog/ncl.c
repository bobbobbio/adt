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

   create(file, tcp_socket);
   ecrash(tcp_bind(port, &tcp_socket));
   ecrash(tcp_listen(&tcp_socket, 0));

   printf("Listening for connections on port %d\n", port);

   create(file, client_conn);
   create(inet_addr, client_addr);
   ecrash(tcp_accept(&tcp_socket, &client_conn, &client_addr));

   aprintf("Got connection from %s\n\n", print(inet_addr, &client_addr));

   create_line_reader(stdin_lr, (struct stream *)file_stdin);

   create(string, buff);
   while (stream_has_more((struct stream *)&client_conn)) {
      create_file_set(fd_set, &client_conn, file_stdin);
      ecrash(file_set_select(&fd_set));

      if (file_set_is_set(&fd_set, &client_conn)) {
         ecrash(stream_read_n_or_less(
            (struct stream *)&client_conn, &buff, 1024));
         printf("%s", string_to_cstring(&buff));
      }

      if (file_set_is_set(&fd_set, file_stdin)) {
         line_reader_get_line(&stdin_lr, &buff);
         string_append_cstring(&buff, "\n");
         ecrash(stream_write((struct stream *)&client_conn, &buff));
      }
   }

   return EXIT_SUCCESS;
}
