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
constexpr size_t SINGLE_LENGTH = UDP_MAX_SIZE - sizeof(unsigned) * 2;
constexpr size_t SLOTS = 2 * 1024;
constexpr size_t INIT_WND = 2;
constexpr size_t HARD_WIND = 20;
constexpr size_t SOFT_WIND = 60;
constexpr size_t NAK_SIZE = 4;
constexpr unsigned int INACTIVE = (unsigned)-1;

// constexpr size_t UDP_BUFFER_SIZE_TMP = (3 * 1024 * 1024);
// constexpr size_t UDP_BUFFER_SEGMENTS = UDP_BUFFER_SIZE_TMP / UDP_DATA_SINGLE;
// constexpr size_t UDP_BUFFER_SIZE = UDP_BUFFER_SEGMENTS * UDP_DATA_SINGLE;
// constexpr size_t UDP_SLOT_COUNT = 8;
// constexpr size_t UDP_FIXED_WIND = 10;

struct Datagram {
  unsigned int seq_index;
  unsigned int wind : 16;
  unsigned int flags : 16;
  char data[SINGLE_LENGTH] __attribute__((aligned(8)));
};
static_assert(sizeof(Datagram) == UDP_MAX_SIZE, "fake");

struct DatagramACK {
  // unsigned int seq[ACK_SIZE * 2 * UDP_SLOT_COUNT];
  unsigned short awnd;
  unsigned short flags;
  unsigned int main_ack; // define it as offset / UDP_DATA_SINGLE
  // if end = 0
  // see as fake
  unsigned int naks[NAK_SIZE];
  size_t bottom_ack() { return naks[0] == INACTIVE ? main_ack : naks[0]; }
  DatagramACK() : main_ack(0) {
    for (auto &x : naks) {
      x = INACTIVE;
    }
  }
  bool isAcked(size_t seq_index) {
    if (seq_index >= main_ack)
      return false;
    for (auto nak : naks) {
      if (seq_index == nak)
        return false;
    }
    return true;
  }
  bool update(size_t seq_index) {
    if (main_ack == seq_index) {
      // if(main_ack >= 216960){
      // cerr << "update";
      // LOG(main_ack);
      // }
      main_ack++;
      return true;
    } else if (main_ack < seq_index) {
      // new ack
      for (auto &nak : naks) {
        if (nak == INACTIVE) {
          nak = seq_index;
          main_ack = seq_index + 1;
          return true;
        } else if (nak == seq_index) {
          return false;
        }
      }
      return false;
    } else {
      // fill old
      for (size_t i = 0; i < NAK_SIZE; ++i) {
        if (naks[i] == seq_index) {
          while (++i < NAK_SIZE) {
            naks[i - 1] = naks[i];
          }
          naks[NAK_SIZE - 1] = -1;
          return true;
        }
      }
      return false;
    }
  }
};
using std::vector;

Jam jam(0.000);
template <size_t max_interval, class T> class Window {
  static_assert(((max_interval - 1) & max_interval) == 0, "fake");

public:
  Window(size_t limit, size_t interval = max_interval)
      : base(0), limit(limit), data(max_interval) {}

  T &operator[](size_t i) {
    if (!(offset_begin() <= i && offset_end() > i)) {
      LOG(offset_begin());
      LOG(i);
      LOG(offset_end());
      assert(false);
    }
    return data[i % max_interval];
  }

  bool empty() { return base >= limit; }
  size_t offset_begin() { return base; }
  size_t offset_end() { return std::min(base + max_interval, limit); }
  void push(size_t n = 1) { base += n; }
  void push_to(size_t beg) { base = std::max(beg, base); }
  T &raw_ptr() {
    assert(base % (max_interval / 2) == 0);
    return data[base % max_interval];
  }

private:
  size_t base;
  size_t limit;
  vector<T> data;
};

#include <deque>
static void receiver(UDP server, FILE *local_file, size_t length) {
  // assume 16 rounds is OK
  bool isFirstRecv = true;
  DatagramACK reply;
  size_t max_seq_index = (length - 1) / SINGLE_LENGTH + 1;
  Window<SLOTS * 2, char[SINGLE_LENGTH]> buffer(max_seq_index);
  DogAddr addr;
  size_t ack_edge = 0;
  // based on packet
  int status;
  size_t write_edge = SLOTS;
  server.set_timeout(1000'000);
  while (true) {
    Datagram send_data;
    auto status = isFirstRecv
                      ? server.recvfrom(&send_data, sizeof(send_data), addr)
                      : server.recv(&send_data, sizeof(send_data));
    if (!status) {
      cerr << "timeout!!";
      LOG(send_data.seq_index);
      continue;
    } else if (isFirstRecv) {
      cerr << "handshake";
      server.connect(addr.get_ip(), addr.get_port());
      isFirstRecv = false;
    }

    // LOG(send_data.seq_index);
    auto isAckValid = reply.update(send_data.seq_index);
    if (isAckValid) {
      COPY(buffer[send_data.seq_index], send_data.data);
      if (reply.bottom_ack() >= write_edge) {
        auto write_length = std::min(length, SINGLE_LENGTH * write_edge);
        write_length -= buffer.offset_begin() * SINGLE_LENGTH;
        ::fwrite(buffer.raw_ptr(), 1, write_length, local_file);
        // LOG(write_edge);
        // LOG(reply.bottom_ack());
        // LOG(write_length);
        auto total_crc = naive_hash(buffer.raw_ptr(), SLOTS * SINGLE_LENGTH);
        LOG(total_crc);
        write_edge = std::min(max_seq_index, write_edge + SLOTS);
        buffer.push(SLOTS);
        if (buffer.empty()) {
          goto endOfRecv;
        }
      }
    } else {
      cerr << "dup!";
      LOG(reply.main_ack);
      LOG(send_data.seq_index);
    }
    server.send(&reply, sizeof(reply));
  }
endOfRecv:
  server.send(&reply, sizeof(reply));
  server.set_timeout(500'000);
  while (true) {
    Datagram send_data;
    auto status = server.recv(&send_data, sizeof(send_data));
    if (!status) {
      break;
    }
    server.send(&reply, sizeof(reply));
  }
  return;
}
struct LostPack {
  size_t seq_index;
  size_t trigger_ack;
};

static void sender(UDP conn, FILE *sending_file, size_t length) {
  DatagramACK total_reply;
  size_t max_seq_index = (length - 1) / SINGLE_LENGTH + 1;
  Window<SLOTS * 2, char[SINGLE_LENGTH]> buffer(max_seq_index);
  std::queue<LostPack> lost_queue;
  size_t send_seq_index = 0;
  conn.set_timeout(500'000);

  int n = ::fread(buffer.raw_ptr(), 1, 2 * SLOTS * SINGLE_LENGTH, sending_file);
  assert(n == 2 * SLOTS * SINGLE_LENGTH);
  {
    auto total_crc = naive_hash(buffer.raw_ptr(), SLOTS * SINGLE_LENGTH);
    LOG(total_crc);
  }
  {
    auto total_crc = naive_hash(buffer.raw_ptr() + SLOTS * SINGLE_LENGTH,
                                SLOTS * SINGLE_LENGTH);
    LOG(total_crc);
  }

  bool isTimeout = false;
  size_t write_edge = std::min(max_seq_index, SLOTS);
  while (true) {
    auto main_ack = total_reply.main_ack;
    // fix lost packet

    if (!lost_queue.empty()) {
      int count = lost_queue.size();
      while (count-- > 0) {
        // time to check lost
        if (!isTimeout && main_ack < lost_queue.front().trigger_ack)
          break;
        auto resent_seq_index = lost_queue.front().seq_index;
        lost_queue.pop();
        if (total_reply.isAcked(resent_seq_index)) {
          continue;
        }
        Datagram send_data;
        send_data.seq_index = (unsigned)(resent_seq_index);
        COPY(send_data.data, buffer[resent_seq_index]);
        conn.send(&send_data, sizeof(send_data));
        lost_queue.push({resent_seq_index, main_ack + 4});
      }
      if (isTimeout) {
        for (size_t index = total_reply.main_ack; index < send_seq_index;
             ++index) {
          //
          Datagram send_data;
          send_data.seq_index = (unsigned)(index);
          COPY(send_data.data, buffer[index]);
          conn.send(&send_data, sizeof(send_data));
          lost_queue.push({index, index + 4});
        }
      }
      isTimeout = false;
    }

    // to avoid congestion
    if (send_seq_index >= max_seq_index ||
        send_seq_index >= main_ack + SOFT_WIND ||
        send_seq_index >= total_reply.bottom_ack() + HARD_WIND) {
      // recv for the godness
      DatagramACK reply;
      auto status = conn.recv(&reply, sizeof(reply));
      if (!status) {
        // timeout
        // emmmmm what the hell...
        cerr << "timeout";
        LOG(main_ack);
        LOG(send_seq_index);
        isTimeout = true;
        // goto first section
        // block wind growth
        continue;
      } else {
        // update reply
        // assume no reordered ack
        total_reply = reply;
        // check if lock can be release
        if (reply.bottom_ack() >= write_edge) {
          int nread = ::fread(buffer.raw_ptr(), 1, SLOTS * SINGLE_LENGTH, sending_file);
          auto total_crc = naive_hash(buffer.raw_ptr(), nread);
          cerr << nread;
          LOG(total_crc);
          buffer.push(SLOTS);
          write_edge += SLOTS;
        }
        if (reply.bottom_ack() >= max_seq_index) {
          return;
        }
      }
      continue;
    }
    // sending main data
    Datagram send_data;
    send_data.seq_index = (unsigned)(send_seq_index);
    COPY(send_data.data, buffer[send_seq_index]);
    conn.send(&send_data, sizeof(send_data));
    send_seq_index++;
  }
}

// std::vector<char> buffer(SINGLE_LENGTH * SLOTS * 1);
// std::vector<size_t> recv_count(SLOTS);
// size_t last_index = 0;
// conn.set_timeout(100'000);
// auto tbeg = dog_timer();
// for (size_t seq_base = 0; seq_base < length;
//      seq_base += (SINGLE_LENGTH * SLOTS)) {
//   size_t seq_end = std::min(length, seq_base + (SINGLE_LENGTH * SLOTS));
//   auto nread =
//       ::fread(buffer.data(), 1, (SINGLE_LENGTH * SLOTS), sending_file);
//   auto total_crc = naive_hash(buffer.data(), seq_end - seq_base);
//   LOG(total_crc);
//   for (size_t seq = seq_base; seq < seq_end;) {
//     size_t seq_index = seq / SINGLE_LENGTH;
//     Datagram send_data;
//     send_data.seq_index = (unsigned)(seq_index);
//     COPY(send_data.data, buffer.data() + seq - seq_base);
//     conn.send(&send_data, sizeof(send_data));
//     DatagramACK ack_data;
//     auto status = conn.recv(&ack_data, sizeof(ack_data));
//     if (!status) {
//       cerr << "pack lost";
//       LOG(seq_index);
//       continue;
//     }
//     if (ack_data.main_ack - 1 == seq_index) {
//       seq += SINGLE_LENGTH;
//       continue;
//     } else {
//       cerr << "ackdup";
//       usleep(1000);
//       LOG(ack_data.main_ack - 1);
//       LOG(seq_index);
//     }
//   }
// }
// cerr << "pertend sending";
// LOG(dog_timer() - tbeg);
static_assert(sizeof(SINGLE_LENGTH) % 8 == 0, "alignment incorrect");
