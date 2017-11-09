#include "file_udp.h"
#include <atomic>
#include <mutex>
#include <thread>

static void receiver(UDP server, FILE *local_file, size_t length);
in_port_t Real_FileUDP::open_receive_port(const string &local_file_path,
                                          size_t length) {
  FILE *local_file = fopen(local_file_path.c_str(), "wb");
  if (local_file == nullptr) {
    cerr << "failed to open file in " << __FUNCTION__ << endl;
  }
  UDP server;
  server.socket();
  server.bind(0);
  std::thread go(
      [](UDP server, FILE *local_file, size_t length) {
        receiver(server, local_file, length);
        server.close();
        fclose(local_file);
      },
      server, local_file, length);
  go.detach();
  return server.getsockname().get_port();
}

static void sender(UDP conn, FILE *sending_file, size_t length);
void Real_FileUDP::send(const string &file_path, in_addr_t server_ip,
                        in_port_t port) {
  FILE *sending_file = fopen(file_path.c_str(), "rb");
  UDP conn;
  conn.connect(server_ip, port);
  size_t length = len(file_path);
  LOG(length);
  std::thread go(
      [](UDP conn, FILE *sending_file, size_t length) {
        sender(conn, sending_file, length);
        conn.close();
        fclose(sending_file);
      },
      conn, sending_file, length);
  go.detach();
}

constexpr size_t MTU = 1500;
constexpr size_t UDP_MAX_SIZE = (MTU - 8) - 20;
constexpr size_t ACK_SIZE = 32;
constexpr size_t UDP_DATA_SINGLE = UDP_MAX_SIZE - sizeof(unsigned) * 3;
constexpr size_t UDP_BUFFER_SIZE_TMP = (16 * 1024 * 1024);
constexpr size_t UDP_BUFFER_SEGMENTS = UDP_BUFFER_SIZE_TMP / UDP_DATA_SINGLE;
constexpr size_t UDP_BUFFER_SIZE = UDP_BUFFER_SEGMENTS * UDP_DATA_SINGLE;
constexpr size_t UDP_SLOT_COUNT = 2;

struct Datagram __attribute__((aligned(8))) {
  unsigned int seq_num; //
  unsigned int wind : 16;
  unsigned int flags : 16;
  char data[UDP_DATA_SINGLE] __attribute__((aligned(8)));
};

struct DatagramACK __attribute__((aligned(8)) {
  // unsigned int seq_num[ACK_SIZE * 2 * UDP_SLOT_COUNT];
  unsigned int seq_num[2]; // define it as offset / UDP_DATA_SINGLE
};

static void receiver(UDP server, FILE *local_file, size_t length) {
  // assume 16 rounds is OK
  bool isFirstRecv = true;
  std::vector<char> buffer(UDP_BUFFER_SIZE * UDP_SLOT_COUNT);
  std::vector<size_t> recv_count(UDP_SLOT_COUNT);
  std::bitset<UDP_BUFFER_SIZE> ack_record[UDP_SLOT_COUNT];
  // std::mutex mu;
  cerr << "pertend receiving";
}

static void sender(UDP conn, FILE *sending_file, size_t length) {
  std::vector<char> buffer(UDP_BUFFER_SIZE * UDP_SLOT_COUNT);
  std::vector<size_t> recv_count(UDP_SLOT_COUNT);
  std::bitset<UDP_BUFFER_SIZE> ack_record[UDP_SLOT_COUNT];
  conn.set_timeout(100'000);
  for (size_t seq_base = 0; seq_base < length; seq_base += UDP_BUFFER_SIZE) {
    size_t seq_end = std::min(length, seq_base + UDP_BUFFER_SIZE);
    ::fread(buffer.data(), 1, UDP_BUFFER_SIZE, sending_file);
    for (size_t seq = seq_base; seq < seq_end; seq += UDP_BUFFER_SIZE) {
      int offset = (seq >> 3);
      Datagram send_data;
      send_data.seq_num = (unsigned)(seq >> 3);
      COPY(send_data.data, buffer.data() + seq);
      conn.send(&send_data, sizeof(send_data));
      DatagramACK ack_data;
      auto status = conn.recv(ack_data, sizeof(send_data));
      if (status < 0) {
        if (errno == EAGAIN) {
          // timeout
          cerr << "got a timeout";
          seq -= UDP_BUFFER_SIZE;
          continue;
        }
      }
    }
  }
  cerr << "pertend sending";
}

static_assert(sizeof(Datagram) == UDP_MAX_SIZE, "WA");
static_assert(sizeof(UDP_DATA_SINGLE) % 8 == 0, "alignment incorrect");
