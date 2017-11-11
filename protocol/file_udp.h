#ifndef DOG_FILE_UDP_H_
#define DOG_FILE_UDP_H_

#include "../include/common.h"
#include "../include/dog_socket.h"
#include <bitset>
#include <cstdio>
#include <queue>
#include <random>
#include <sys/stat.h>
#include <vector>

class Jam {
public:
  Jam(double p) : p(p), eng(2333), distr(0, 1) {}
  bool lost() { return distr(eng) < p; }

private:
  const double p;
  std::default_random_engine eng;
  std::uniform_real_distribution<double> distr;
};

class UDP : public Communication {
public:
  UDP() = default;
  UDP(int fd) : Communication(fd) {}
  void socket() {
    this->fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    ERROR_EXIT(fd);
  }

  void set_timeout(size_t usec) {
    timeval t;
    t.tv_sec = 0;
    t.tv_usec = usec;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t));
  }

  void bind(in_port_t port) {
    auto addr = create_addr(INADDR_ANY, port);
    int status = ::bind(fd, SAP(addr), addrlen());
    ERROR_EXIT(status);
  }

  void connect(in_addr_t server_ip, in_port_t port) {
    auto addr = create_addr(INADDR_ANY, port);
    ::connect(fd, SAP(addr), addrlen());
  }

  void close() { ::close(fd); }
  void send(void *buf, size_t size) {
    extern Jam jam;
    if (jam.lost()) {
      return;
    }
    ::send(fd, buf, size, 0);
  }
  bool recv(void *buf, size_t size) {
    int status = ::recv(fd, buf, size, 0);
    if (status < 0) {
      if (errno != EAGAIN) {
        LOG(strerror(errno));
        assert(errno == EAGAIN);
      }
      return false;
    }
    return true;
  }
  bool recvfrom(void *buf, size_t size, DogAddr &addr) {
    // DogAddr addr;
    auto len = addrlen();
    int status = ::recvfrom(fd, buf, size, 0, SAP(addr.raw()), &len);
    if (status < 0) {
      assert(errno == EAGAIN);
      return false;
    }
    return true;
  }
};

class Real_FileUDP {
public:
  static in_port_t open_receive_port(const string &local_file_path,
                                     size_t length);
  static size_t len(const string &file_path) {
    struct stat64 file_status;
    int status = stat64(file_path.c_str(), &file_status);
    ERROR_EXIT(status);
    return file_status.st_size;
  }
  static void send(const string &file_path, in_addr_t server_ip,
                   in_port_t port);

private:
};

using FileUDP = Real_FileUDP;
#endif
