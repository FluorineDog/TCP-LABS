#ifndef DOG_COMMON_H_
#define DOG_COMMON_H_

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

// extern "C" {
#include <arpa/inet.h>
#include <map>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <tuple>
#include <unistd.h>

// }
#include <sqlite/connection.hpp>
#include <sqlite/execute.hpp>
#include <sqlite/query.hpp>

using std::cerr;
using std::cin;
using std::string;
using std::cout;
using std::endl;
using std::tuple;
using boost::shared_ptr;
using std::map;

// #define COPY(dest, src) strncpy(dest, src, 32)
template <int N> inline void COPY(char (&dest)[N], const string &src) {
  memcpy(dest, src.c_str(), N);
}

template <int N> inline void COPY(char (&dest)[N], const char *src) {
  memcpy(dest, src, N);
}



#define ERROR_EXIT(status)                                                     \
  if (status == -1) {                                                          \
    cerr << "failed to " << __FUNCTION__ << endl;                              \
    exit(-1);                                                                  \
  }

#define LOG(expr) std::cerr << "*** " #expr << " <*> " << (expr) << endl

constexpr in_addr_t SERVER_IP = 0x7f'00'00'01; // 127.0.0.1
constexpr in_port_t SERVER_PORT = 30000;
// constexpr int BUFFER_SIZE = 1024;
constexpr int LISTENQ = 1024;
constexpr size_t TRANS_BLOCK_SIZE = 64 * 1024 * 1024;
constexpr size_t TRANS_BLOCK_SIZE_EXP = 26;
inline double dog_timer() {
  timespec tmp;
  clock_gettime(CLOCK_MONOTONIC, &tmp);
  return tmp.tv_sec + tmp.tv_nsec / 1.0e9;
}
#endif // DOG_COMMON_H_
