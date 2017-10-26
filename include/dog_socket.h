#ifndef DOG_SOCKET_H_
#define DOG_SOCKET_H_

#include "common.h"
#include <cstring>

class TCP {
private:
public:
  // fake;
  TCP() {
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
      cerr << "failed to create sockets" << endl;
      exit(-1);
    }
  }
  void connect(in_addr_t server_ip, in_port_t port) {
    create_addr(server_ip, port);
    int errno = ::connect(fd, get_addr(), sizeof(sockaddr_in));
    if (errno == -1) {
      cerr << "failed to connect" << endl;
      exit(-1);
    }
  }
  void bind() {

  }
  void listen();

private:
  void create_addr(in_addr_t server_Port, in_port_t port) {
    memset(&addr, 0, sizeof(addr));
    addr.sin_addr.s_addr = htonl(server_Port);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
  }
  sockaddr *get_addr() { return (sockaddr *)&addr; }
  sockaddr_in addr;
  int fd;
};

class UDP {
public:
  // fake;
private:
  // fake:
};

class Comm {
public:
  // fake;
private:
  // fake:
};
#endif // DOG_SOCKET_H_
