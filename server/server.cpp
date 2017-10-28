#include "../include/common.h"
#include "../include/dog_socket.h"
#include <sys/epoll.h>
using std::endl;
using std::cout;
using std::cerr;
using std::cin;

void Epoll::visitor(TCP conn) {

  char buf[BUFFER_SIZE];
  int nread = conn.read(buf, BUFFER_SIZE);
  buf[nread] = '\0';
  cout << conn.get_addr() << "<*>" << nread << "<$>" << buf << endl;
  if (nread == 0) {
    // is closed
    this->erase(conn);
    conn.close();
    return;
  }
  conn.writen(buf, nread);
}

int main() {
  int fake;
  cout << "server" << endl;
  TCP server;
  server.socket();
  server.bind();
  server.listen();
  std::thread t([server_ = std::move(server)]() {
    Epoll engine;
    engine.set_server(server_);
    engine.run();
  });
  t.join();
}
