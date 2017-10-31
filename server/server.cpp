#include "../include/common.h"
#include "../include/dog_socket.h"
#include "../protocol/dataflow.h"

#include <sys/epoll.h>
using std::endl;
using std::cout;
using std::cerr;
using std::cin;

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
  // char buf[BUFFER_SIZE];
  // int nread = conn.read(buf, BUFFER_SIZE);
  // buf[nread] = '\0';
  // cout << conn.get_addr() << "<*>" << nread << "<$>" << buf << endl;
  // if (nread == 0) {
  //   // is closed
  //   this->erase(conn);
  //   conn.close();
  //   return;
  // }
  // conn.writen(buf, nread);
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
