#include "file_udp.h"
#include "../include/security.h"
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
  conn.socket();
  conn.bind(0);
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
constexpr size_t UDP_DATA_SINGLE = UDP_MAX_SIZE - sizeof(unsigned) * 2;
constexpr size_t UDP_BUFFER_SIZE_TMP = (4 * 1024 * 1024);
constexpr size_t UDP_BUFFER_SEGMENTS = UDP_BUFFER_SIZE_TMP / UDP_DATA_SINGLE;
constexpr size_t UDP_BUFFER_SIZE = UDP_BUFFER_SEGMENTS * UDP_DATA_SINGLE;
constexpr size_t UDP_SLOT_COUNT = 2;
constexpr size_t UDP_FIXED_WIND = 10;

struct Datagram {
  unsigned int seq; //
  unsigned int wind : 16;
  unsigned int flags : 16;
  char data[UDP_DATA_SINGLE] __attribute__((aligned(8)));
};

struct DatagramACK {
  // unsigned int seq[ACK_SIZE * 2 * UDP_SLOT_COUNT];
  unsigned int seq[2]; // define it as offset / UDP_DATA_SINGLE
};

static void receiver(UDP server, FILE *local_file, size_t length) {
  // assume 16 rounds is OK
  bool isFirstRecv = true;
  std::vector<char> buffer(UDP_BUFFER_SIZE * UDP_SLOT_COUNT);
  std::vector<size_t> recv_count(UDP_SLOT_COUNT);
  std::bitset<UDP_BUFFER_SIZE> ack_record[UDP_SLOT_COUNT];
  // std::mutex mu;
  // LOG(server.getsockname().get_ip());
  // LOG(server.getsockname().get_port());
  cerr << "pertend receiving";
  DogAddr addr;
  server.set_timeout(500'000);
  for (size_t seq_base = 0; seq_base < length; seq_base += UDP_BUFFER_SIZE) {
    size_t seq_end = std::min(length, seq_base + UDP_BUFFER_SIZE);
    for (size_t seq = seq_base; seq < seq_end;) {
      Datagram send_data;
      int status;
      if (isFirstRecv) {
        // cerr << "handshake";
        status = server.recvfrom(&send_data, sizeof(send_data), addr);
      } else {
        status = server.recv(&send_data, sizeof(send_data));
      }
      if (!status) {
        cerr << "timeout!!";
        LOG(seq);
        continue;
      }
      if (seq != send_data.seq) {
        cerr << "dup!!";
        LOG(seq);
        LOG(send_data.seq);
        // continue;
      }
      auto crc = naive_hash(send_data.data, UDP_DATA_SINGLE);
      ::memcpy(buffer.data() + seq - seq_base, send_data.data,
                UDP_DATA_SINGLE);
      // feedback
      DatagramACK ack_data;
      ack_data.seq[0] = send_data.seq;
      if (isFirstRecv) {
        server.connect(addr.get_ip(), addr.get_port());
        // LOG(server.getpeername().get_ip());
        // LOG(server.getpeername().get_port());
        // cerr << "fake connection" << endl;
        isFirstRecv = false;
      }
      server.send(&ack_data, sizeof(ack_data));
      // cerr << "receiving with ";
      // LOG(seq);
      // LOG(crc);
      seq += UDP_DATA_SINGLE;
    }
    // cerr << "fake writing" << endl;
    auto nwrite = ::fwrite(buffer.data(), 1, seq_end - seq_base, local_file);
    // assert(nwrite + seq_base == seq_end);
    // auto total_crc = naive_hash(buffer.data(), seq_end - seq_base);
    // LOG(total_crc);
  }
}

static void sender(UDP conn, FILE *sending_file, size_t length) {
  std::vector<char> buffer(UDP_BUFFER_SIZE * UDP_SLOT_COUNT);
  std::vector<size_t> recv_count(UDP_SLOT_COUNT);
  std::bitset<UDP_BUFFER_SIZE> ack_record[UDP_SLOT_COUNT];
  // LOG(conn.getsockname().get_ip());
  // LOG(conn.getsockname().get_port());
  // LOG(conn.getpeername().get_ip());
  // LOG(conn.getpeername().get_port());
  // cerr << "fake sender";
  conn.set_timeout(500'000);
  auto tbeg = dog_timer();
  for (size_t seq_base = 0; seq_base < length; seq_base += UDP_BUFFER_SIZE) {
    size_t seq_end = std::min(length, seq_base + UDP_BUFFER_SIZE);
    // cerr << "fake reading" << endl;
    auto nread = ::fread(buffer.data(), 1, UDP_BUFFER_SIZE, sending_file);
    // assert(nread + seq_base == seq_end);
    // auto total_crc = naive_hash(buffer.data(), seq_end - seq_base);
    // LOG(total_crc);
    for (size_t seq = seq_base; seq < seq_end;) {
      Datagram send_data;
      send_data.seq = (unsigned)(seq);
      COPY(send_data.data, buffer.data() + seq - seq_base);
      // auto crc = naive_hash(send_data.data, UDP_DATA_SINGLE);
      conn.send(&send_data, sizeof(send_data));
      DatagramACK ack_data;
      auto status = conn.recv(&ack_data, sizeof(ack_data));
      if (ack_data.seq[0] != seq) {
        LOG(ack_data.seq[0]);
        LOG(seq);
        continue;
      }
      // cerr << "sending with ";
      // LOG(seq);
      // LOG(crc);
      seq += UDP_DATA_SINGLE;
    }
  }
  cerr << "pertend sending";
  LOG(dog_timer() - tbeg);
}

static_assert(sizeof(Datagram) == UDP_MAX_SIZE, "WA");
static_assert(sizeof(UDP_DATA_SINGLE) % 8 == 0, "alignment incorrect");
