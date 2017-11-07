#include "file_udp.h"

static void receiver(UDP server, FILE *file);

in_port_t Real_FileUDP::open_receive_port(const string &local_file_path,
                                          size_t length) {
  FILE *file = fopen(local_file_path.c_str(), "wb");
  if (file == nullptr) {
    cerr << "failed to open file in " << __FUNCTION__ << endl;
  }
  UDP server;
  server.socket();
  server.bind(0);
  std::thread go(
      [](UDP server, FILE *file) {
        receiver(server, file);
        server.close();
        fclose(file);
      },
      server, file);
  go.detach();
  return server.getsockname().get_port();
}
static void sender(UDP conn, FILE *local_file);
void Real_FileUDP::send(const string &file_path, in_addr_t server_ip,
                        in_port_t port) {
  FILE *file = fopen(file_path.c_str(), "rb");
  UDP conn;
  conn.connect(server_ip, port);
  size_t length = len(file_path);
  LOG(length);
  std::thread go(
      [](UDP conn, FILE *local_file) {
        sender(conn, local_file);
        conn.close();
        fclose(local_file);
      },
      conn, local_file);
  go.detach();
}

static void receiver(UDP server, FILE *file) {
  // input server && file which is ready
  cerr << "pertend receiving";
  l
}

static void sender(UDP conn, FILE *local_file) { cerr << "pertend sending"; }