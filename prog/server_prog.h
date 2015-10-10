// Copyright 2015 Remi Bernotavicius. All right reserved.

#include <adt.h>
#include <extyp/network.h>

void
server_main(int port);

// To be implemented by including file
void
serve_client(struct socket *client_conn, struct inet_addr *client_addr);
