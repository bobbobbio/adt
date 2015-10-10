// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <prog/server_prog.h>

#include <stdtyp/auto_free_pool.h>
#include <stdtyp/file.h>
#include <stdtyp/signal.h>
#include <stdtyp/threading.h>

fptr_define_void(serve_client, struct socket *, struct inet_addr *);

struct auto_free_pool *g_pool = NULL;

static void
cleanup_and_exit(void)
{
   if (g_pool != NULL)
      auto_free_pool_free(g_pool);
   exit(1);
}

void
server_main(int port)
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

   // Listen on given port for incoming connections.
   ecrash(socket_bind(tcp_socket, port));

   // The second argument here is how many connections to queue in the kernel
   // before dropping connections.
   ecrash(socket_listen(tcp_socket, 10));

   printf("Server started on port %d\n", port);
   printf("Press Enter to exit...\n");

   create(thread_pool, tp);

   while (true) {
      create_file_set(fd_set, &tcp_socket->file, file_stdin);
      ecrash(file_set_select(&fd_set));

      if (file_set_is_set(&fd_set, &tcp_socket->file)) {
         // XXX We are potentially leaking the client connection here if we exit
         // on a signal, but it doesn't matter so much, it is an unknown port.
         struct socket *client_conn = socket_new();
         struct inet_addr *client_addr = inet_addr_new();

         // Accept a connection and serve it's request asynchronously.
         ecrash(socket_accept(tcp_socket, client_conn, client_addr));
         thread_pool_run(&tp, serve_client, client_conn, client_addr);
      }

      if (file_set_is_set(&fd_set, file_stdin)) {
         printf("Server exiting...\n");
         break;
      }

   }

   // XXX: somehow wake up all the sleeping threads and make them exit...

   thread_pool_join(&tp);

   auto_free_pool_free(g_pool);
}
