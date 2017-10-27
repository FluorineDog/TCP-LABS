#include "../include/common.h"
#include "../include/dog_socket.h"
#include <sys/epoll.h>
using std::endl;
using std::cout;
using std::cerr;
using std::cin;
void workload(TCP conn) {
  cerr << "thread created" << endl;
  char buf[BUFFER_SIZE];
  while (true) {
    int nread = conn.read(buf, BUFFER_SIZE);
    buf[nread] = '\0';
    cout << conn << "<*>" << nread << "<$>" << buf << endl;
    if(nread == 0 ){
      break;
    }
		conn.writen(buf, nread);
  }
  conn.close();
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
