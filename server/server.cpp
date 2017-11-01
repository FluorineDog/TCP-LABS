#include "../include/common.h"
#include "../include/dog_socket.h"
#include "../protocol/dataflow.h"

#include <sys/epoll.h>
using std::endl;
using std::cout;
using std::cerr;
using std::cin;
#include <map>
#include <tuple>
using std::map;
using std::string;
using std::tuple;

sqlite::connection sql("data/server.db");

map<TCP, tuple<string>> incoming_conn;
void Epoll::visitor(TCP conn) {
  cerr << "try getting type";
  auto data = RawData::get_type(conn);
  if (data == nullptr) {
    // client is closed
    cerr << "client is down" << endl;
    this->erase(conn);
    conn.close();
    return;
  }
  data->read_data(conn);
  data->action(conn);
}

int main() {
  int fake;
  cout << "server" << endl;
  TCP server;
  server.socket();
  server.bind();
  server.listen();
  std::thread t(
      [](TCP server) {
        Epoll engine;
        engine.set_server(std::move(server));
        engine.run();
      },
      server);
  t.join();
}
