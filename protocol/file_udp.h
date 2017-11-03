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
//   void bind(in_port_t port){
//     auto addr = create_addr(INADDR_ANY, port);
//     int status = ::bind(0);
//     ERROR_EXIT(status);
//   }
//   void connect(auto server_ip, in_port_t){

//   }
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
#include <sys/stat.h>
class Fake_File_UDP {
public:
  void receive(const string &file_path, size_t length_) {
    file = fopen(file_path.c_str(), "wb");
    if (file == nullptr) {
      cerr << "failed to open file in " << __FUNCTION__ << endl;
    }
    this->length = length_;
    std::unique_ptr<char[]> buf(new char[TRANS_BLOCK_SIZE]);
    TCP server;
    server.socket();
    server.bind(0);
    server.listen();
    TCP conn = server.accept();
    for (int offset = 0; offset < length;) {
      int sz = conn.read(buf.get(), TRANS_BLOCK_SIZE);
      offset += sz;
      fwrite(buf.get(), sz, 1, file);
      cerr << "recv block sz" << sz << endl;
    }
  }

  static long len(const string &file_path) {
    struct stat64 file_status;
    int status = stat64(file_path.c_str(), &file_status);
    ERROR_EXIT(status);
    return file_status.st_size;
  }

  void ready_file(string file_path) {
    file = fopen(file_path.c_str(), "rb");
    length = len(file_path);
    return;
  }

  void send(auto server_ip, in_port_t port) {
    //
    TCP conn;
    conn.socket();
    conn.connect(server_ip, port);
    std::unique_ptr<char[]> buf(new char[TRANS_BLOCK_SIZE]);
    for (int offset = 0; offset < length;) {
      int sz = fread(buf.get(), TRANS_BLOCK_SIZE, 1, file);
      conn.writen(buf.get(), sz);
      offset += sz;
      cerr << "trans block " << sz << endl;
    }
  }

private:
  FILE *file;
  size_t length;
};
using File_UDP = Fake_File_UDP;

#endif