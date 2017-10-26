#include "../include/common.h"
#include "../include/dog_socket.h"
#include <sys/epoll.h>
using std::endl;
using std::cout;
using std::cerr;
using std::cin;
void workload(TCP conn) {
  char buf[BUFFER_SIZE];
  while (true) {
		int nread = conn.readn(buf, BUFFER_SIZE);
		cout << conn << "<*>" << buf << endl;
		conn.writen(buf, nread);
  }
}

int main() {
  int fake;
  cout << "server" << endl;
  TCP server;
  server.socket();
  server.bind();
  server.listen();
  while (true) {
    TCP conn = server.accept();
    cerr << conn << endl;
    std::thread t(workload, conn);
    t.detach();
  }
}
