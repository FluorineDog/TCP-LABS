#ifndef DOG_FILE_UDP_H_
#define DOG_FILE_UDP_H_

#include "../include/common.h"
#include "../include/dog_socket.h"
#include <bitset>
#include <cstdio>
#include <queue>
#include <sys/stat.h>
#include <vector>

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
  void send(void *buf, size_t size) { ::send(fd, buf, size, 0); }
  bool recv(void *buf, size_t size) {
    int status = ::recv(fd, buf, size, 0);
    if (status < 0) {
      assert(errno == EAGAIN);
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

// class Fake_FileUDP {
// public:
//   static in_port_t open_receive_port(const string &local_file_path,
//                                      size_t length) {
//     FILE *file = fopen(local_file_path.c_str(), "wb");
//     if (file == nullptr) {
//       cerr << "failed to open file in " << __FUNCTION__ << endl;
//     }
//     TCP server;
//     server.socket();
//     server.bind(0);
//     server.listen();
//     std::thread t(
//         [](TCP server, size_t length, FILE *file) {
//           std::unique_ptr<char[]> buf(new char[TRANS_BLOCK_SIZE]);
//           TCP conn = server.accept();
//           double t_beg = dog_timer();
//           server.close();
//           for (size_t offset = 0; offset < length;) {
//             size_t sz = conn.read(buf.get(),
//                                   std::min(TRANS_BLOCK_SIZE, length -
//                                   offset));
//             offset += sz;
//             fwrite(buf.get(), 1, sz, file);
//             cerr << "recv block sz " << sz << endl;
//             LOG(offset);
//           }
//           fclose(file);
//           double t_end = dog_timer();
//           LOG(t_end - t_beg);
//           conn.close();
//         },
//         server, length, file);
//     t.detach();
//     return server.getsockname().get_port();
//   }

//   static size_t len(const string &file_path) {
//     struct stat64 file_status;
//     int status = stat64(file_path.c_str(), &file_status);
//     ERROR_EXIT(status);
//     return file_status.st_size;
//   }

//   static void send(const string &file_path, in_addr_t server_ip,
//                    in_port_t port) {
//     FILE *file = fopen(file_path.c_str(), "rb");
//     size_t length = len(file_path);
//     TCP conn;
//     conn.socket();
//     conn.connect(server_ip, port);
//     std::unique_ptr<char[]> buf(new char[TRANS_BLOCK_SIZE]);
//     for (size_t offset = 0; offset < length;) {
//       size_t sz = fread(buf.get(), 1, TRANS_BLOCK_SIZE, file);
//       conn.writen(buf.get(), sz);
//       offset += sz;
//       cerr << "trans block " << sz;
//       LOG(offset);
//     }
//     conn.close();
//   }

// private:
//   // FILE *file;
//   // size_t length;
// };

using FileUDP = Real_FileUDP;
#endif
