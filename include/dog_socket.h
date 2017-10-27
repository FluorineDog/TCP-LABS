#ifndef DOG_SOCKET_H_
#define DOG_SOCKET_H_

#include "common.h"

class TCP {
private:
public:
  // fake;
  TCP() = default;
  TCP(int fd, sockaddr_in addr) : fd(fd), addr(addr) {}
  void socket() {
    fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
      cerr << "failed to create sockets" << endl;
      exit(-1);
    }
  }

  void connect(const char *server_ip, in_port_t port) {
    create_addr(server_ip, port);
    int status = ::connect(fd, SAP(addr), addrlen());
    if (status == -1) {
      cerr << "failed to connect" << endl;
      exit(-1);
    }
  }
  void bind() {
    create_addr(INADDR_ANY, SERVER_PORT);
    int status = ::bind(fd, SAP(addr), addrlen());
    if (status == -1) {
      cerr << "failed to " << __FUNCTION__ << endl;
      exit(-1);
    }
  }
  void listen() {
    int status = ::listen(fd, LISTENQ);
    if (status == -1) {
      cerr << "failed to " << __FUNCTION__ << endl;
      exit(-1);
    }
  }

  // return new TCP bindings
  TCP accept() {
    sockaddr_in cli_addr;
    auto cli_addrlen = addrlen();
    auto connfd = ::accept(fd, SAP(cli_addr), &cli_addrlen);
    if (connfd == -1) {
      cerr << "failed to " << __FUNCTION__ << endl;
      exit(-1);
    }
    if (cli_addrlen != addrlen()) {
      cerr << "no support for ipv6" << endl;
      exit(-1);
    }
    return TCP(connfd, cli_addr);
  }
  int read(char *buf, size_t maxN) {
    ssize_t nread = ::read(fd, buf, maxN);
    if (nread == -1) {
      cerr << "failed to " << __FUNCTION__ << endl;
      exit(-1);
    }
    if(nread == 0){
      // EOF is reached
      return 0;
    }
    return nread;
  }
  int readn(char *buf, size_t n) {
    int raw_n = n;
    while (n > 0) {
      ssize_t nread = ::read(fd, buf, n);
      if (nread == -1) {
        cerr << "failed to " << __FUNCTION__ << endl;
        exit(-1);
      }
      if (nread == 0) {
        // EOF reached
        break;
      }
      buf += nread;
      n -= nread;
    }
    return raw_n - n;
  }
  void writen(char *buf, size_t n) {
    while (n > 0) {
      ssize_t nread = ::write(fd, buf, n);
      if (nread == -1) {
        cerr << "failed to " << __FUNCTION__ << endl;
        exit(-1);
      }
      if (nread == 0) {
        // EOF reached
        break;
      }
      buf += nread;
      n -= nread;
    }
  }
  void close() {
    int status = ::close(fd);
    if (status == -1) {
      cerr << "failed to " << __FUNCTION__ << endl;
      exit(-1);
    }
  }
  friend std::ostream &operator<<(std::ostream &out, const TCP &conn) {
    // buggy
    char ip_buf[36];
    ::inet_ntop(AF_INET, &conn.addr.sin_addr, ip_buf, conn.addrlen());
    return out << ip_buf << ":" << conn.addr.sin_port;
  }

private:
  void create_addr(const char *server_ip, in_port_t port) {
    ::memset(&addr, 0, sizeof(addr));
    inet_pton(server_ip, addr.sin_addr);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
  }

  void create_addr(in_addr_t server_ip, in_port_t port) {
    ::memset(&addr, 0, sizeof(addr));
    addr.sin_addr.s_addr = ::htonl(server_ip);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
  }

  static void inet_pton(const char *server_ip, in_addr &ip) {
    int status = ::inet_pton(AF_INET, server_ip, &ip);
    if (status != 1) {
      cerr << "failed to " << __FUNCTION__ << endl;
      exit(-1);
    }
  }
  static sockaddr *SAP(sockaddr_in &addr) { return (sockaddr *)&addr; }
  static socklen_t addrlen() { return sizeof(sockaddr_in); }

private:
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
