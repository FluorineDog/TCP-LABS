#ifndef DOG_FILE_UDP_H_
#define DOG_FILE_UDP_H_

#include "../include/common.h"
#include "../include/dog_socket.h"
#include <cstdio>

// class UDP {
// public:
//   void socket() {
//     int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
//     ERROR_EXIT(fd);
//   }
//   void bind(in_port_t port) {
//     auto addr = create_addr(INADDR_ANY, port);
//     int status = ::bind(0);
//     ERROR_EXIT(status);
//   }
//   void connect(auto server_ip, in_port_t) {}

// private:
//   static sockaddr_in create_addr(const char *server_ip, in_port_t port) {
//     sockaddr_in addr;
//     ::memset(&addr, 0, sizeof(addr));
//     ::inet_pton(server_ip, addr.sin_addr);
//     addr.sin_port = htons(port);
//     addr.sin_family = AF_INET;
//     return addr;
//   }

//   static sockaddr_in create_addr(in_addr_t server_ip, in_port_t port) {
//     sockaddr_in addr;
//     ::memset(&addr, 0, sizeof(addr));
//     addr.sin_addr.s_addr = ::htonl(server_ip);
//     addr.sin_port = htons(port);
//     addr.sin_family = AF_INET;
//     return addr;
//   }
//   int fd;
// };

// class Real_FileUDP {
// public:
//   static in_port_t open_receive_port(const string &local_file_path,
//                                      size_t length);
//   static size_t len(const string &file_path) {
//     struct stat64 file_status;
//     int status = stat64(file_path.c_str(), &file_status);
//     ERROR_EXIT(status);
//     return file_status.st_size;
//   }
//   size_t size() { return length; }
//   void prepare_send(string file_path);
//   void send(auto server_ip, in_port_t port);

// private:
//   size_t length;
//   string file_length;
// };

#include <sys/stat.h>
class Fake_FileUDP {
public:
  static in_port_t open_receive_port(const string &local_file_path,
                                     size_t length) {
    FILE *file = fopen(local_file_path.c_str(), "wb");
    if (file == nullptr) {
      cerr << "failed to open file in " << __FUNCTION__ << endl;
    }
    TCP server;
    server.socket();
    server.bind(0);
    server.listen();
    std::thread t(
        [](TCP server, size_t length, FILE *file) {
          std::unique_ptr<char[]> buf(new char[TRANS_BLOCK_SIZE]);
          TCP conn = server.accept();
          double t_beg = dog_timer();
          server.close();
          for (size_t offset = 0; offset < length;) {
            size_t sz = conn.read(buf.get(),
                                  std::min(TRANS_BLOCK_SIZE, length - offset));
            offset += sz;
            fwrite(buf.get(), 1, sz, file);
            cerr << "recv block sz " << sz << endl;
            LOG(offset);
          }
          fclose(file);
          double t_end = dog_timer();
          LOG(t_end - t_beg);
          conn.close();
        },
        server, length, file);
    t.detach();
    return server.getsockname().get_port();
  }

  static size_t len(const string &file_path) {
    struct stat64 file_status;
    int status = stat64(file_path.c_str(), &file_status);
    ERROR_EXIT(status);
    return file_status.st_size;
  }

  void prepare_send(string file_path) {
    file = fopen(file_path.c_str(), "rb");
    length = len(file_path);
    static_assert(sizeof(size_t) == 8, "fuck size_t");
    return;
  }
  size_t size() { return length; }
  void send(auto server_ip, in_port_t port) {
    //
    TCP conn;
    conn.socket();
    conn.connect(server_ip, port);
    std::unique_ptr<char[]> buf(new char[TRANS_BLOCK_SIZE]);
    for (size_t offset = 0; offset < length;) {
      size_t sz = fread(buf.get(), 1, TRANS_BLOCK_SIZE, file);
      conn.writen(buf.get(), sz);
      offset += sz;
      cerr << "trans block " << sz;
      LOG(offset);
    }
    conn.close();
  }
private:
  FILE *file;
  size_t length;
};
using FileUDP = Fake_FileUDP;

#endif