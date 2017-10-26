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
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
// }

using std::cerr;
using std::cout;
using std::endl;
const char* SERVER_IP = "127.0.0.1";
constexpr in_port_t SERVER_PORT = 30000;
constexpr int BUFFER_SIZE = 1024;
constexpr int LISTENQ = 1024;
#endif // DOG_COMMON_H_
