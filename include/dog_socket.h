#ifndef DOG_SOCKET_H_
#define DOG_SOCKET_H_

#include "common.h"
#include <functional>
#include <sys/epoll.h>

class TCP {
private:
public:
  // TCP(TCP&&) = default;
  // TCP(const TCP&) = delete;
  // TCP& operator=(TCP&&) = default;
  // TCP& operator=(const TCP&) = delete;
  // fake;
  TCP() = default;
  TCP(int fd) : fd(fd) {}
  void socket() {
    fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
      cerr << "failed to create sockets" << endl;
      exit(-1);
    }
  }
  sockaddr_in getsockname() {
    // std::pair<sockaddr_in, in_port_t> ip_port;
    sockaddr_in addr;
    auto len = addrlen();
    int status = ::getsockname(fd, SAP(addr), &len);
    if (status == -1) {
      cerr << "failed to " << __FUNCTION__ << endl;
      exit(-1);
    }
    return addr;
  }

  sockaddr_in getpeername() {
    // std::pair<sockaddr_in, in_port_t> ip_port;
    sockaddr_in addr;
    auto len = addrlen();
    int status = ::getpeername(fd, SAP(addr), &len);
    if (status == -1) {
      cerr << "failed to " << __FUNCTION__ << endl;
      exit(-1);
    }
    return addr;
  }

  void connect(const auto server_ip, in_port_t port) {
    // connect to server
    auto addr = create_addr(server_ip, port);
    int status = ::connect(fd, SAP(addr), addrlen());
    if (status == -1) {
      cerr << "failed to connect" << endl;
      exit(-1);
    }
  }

  void bind(in_port_t port) {
    // bind to given port
    auto addr = create_addr(INADDR_ANY, port);
    int status = ::bind(fd, SAP(addr), addrlen());
    if (status == -1) {
      cerr << "failed to " << __FUNCTION__ << endl;
      exit(-1);
    }
  }
  void listen() {
    // change to passive mode
    int status = ::listen(fd, LISTENQ);
    if (status == -1) {
      cerr << "failed to " << __FUNCTION__ << endl;
      exit(-1);
    }
  }

  TCP accept() {
    // return new TCP bindings
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
    return TCP(connfd);
  }

  int read(void *buf, size_t maxN) {
    ssize_t nread = ::read(fd, buf, maxN);
    if (nread == -1) {
      cerr << "failed to " << __FUNCTION__ << endl;
      exit(-1);
    }
    if (nread == 0) {
      // EOF is reached
      return 0;
    }
    return nread;
  }

  int readn(void *buf_, size_t n) {
    cerr << "wanna receive" << n << endl;
    char *buf = (char *)buf_;
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
    cerr << "receiving " << n << "bytes" << endl;
    return raw_n - n;
  }

  void writen(const void *buf_, size_t n) {
    cerr << "sending " << n << " bytes" << endl;
    auto buf = (const char *)buf_;
    while (n > 0) {
      ssize_t nread = ::write(fd, buf, n);
      cerr << "nread" << nread << endl;
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

  // int get_raw_fd() { return fd; }
  operator int() { return fd; }

  sockaddr_in get_addr() const {
    sockaddr_in addr;
    socklen_t len = addrlen();
    ::getpeername(fd, SAP(addr), &len);
    return addr;
  }
  friend std::ostream &operator<<(std::ostream &out, const sockaddr_in &addr);

private:
  static sockaddr_in create_addr(const char *server_ip, in_port_t port) {
    sockaddr_in addr;
    ::memset(&addr, 0, sizeof(addr));
    inet_pton(server_ip, addr.sin_addr);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    return addr;
  }

  static sockaddr_in create_addr(in_addr_t server_ip, in_port_t port) {
    sockaddr_in addr;
    ::memset(&addr, 0, sizeof(addr));
    addr.sin_addr.s_addr = ::htonl(server_ip);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    return addr;
  }

  static void inet_pton(const char *server_ip, in_addr &ip) {
    int status = ::inet_pton(AF_INET, server_ip, &ip);
    if (status != 1) {
      cerr << "failed to " << __FUNCTION__ << endl;
      exit(-1);
    }
  }
  static const sockaddr *SAP(const sockaddr_in &addr) {
    return (const sockaddr *)&addr;
  }

  static sockaddr *SAP(sockaddr_in &addr) { return (sockaddr *)&addr; }
  static socklen_t addrlen() { return sizeof(sockaddr_in); }

private:
  int fd;
};

class Epoll {
public:
  Epoll() : server(-1) { epollfd = epoll_create1(0); }
  void set_server(TCP server_) {
    this->server = server_;
    int status = insert(server);
    if (status == -1) {
      cerr << "error at " << __FUNCTION__ << endl;
      exit(-1);
    }
  }

  void run() {
    while (true) {
      epoll_event event;
      int event_count = epoll_wait(epollfd, &event, 1, -1);
      cerr << "getting event" << endl;
      if (event_count != 1) {
        cerr << "error at " << __FUNCTION__ << endl;
        exit(-1);
      }

      if (event.data.fd == server) {
        // in coming connection
        insert(server.accept());
        cerr << "fuck server incoming" << endl;
      } else {
        visitor(event.data.fd);
      }
    }
  }
  void visitor(TCP conn);
  int erase(int fd) {
    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    return epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &event);
  }

  int insert(int fd) {
    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    return epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
  }

private:
  TCP server;
  int epollfd;
};

inline std::ostream &operator<<(std::ostream &out, const sockaddr_in &addr) {
  // buggy
  char ip_buf[36];
  ::inet_ntop(AF_INET, &addr.sin_addr, ip_buf, sizeof(ip_buf));
  return out << ip_buf << ":" << addr.sin_port;
  return out;
}

#endif // DOG_SOCKET_H_
