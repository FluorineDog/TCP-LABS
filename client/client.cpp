#include "../include/common.h"
#include "../include/dog_socket.h"
#include "../protocol/dataflow.h"
#include "../protocol/file_udp.h"

#include <mutex>
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
struct Global {
  string account;
  string passwd;
  TCP client;
  TCP self_server;
  map<string, TCP> lookup;
  Epoll engine;
} global;
std::mutex send_mutex;

void registion(TCP conn) {
  Registion::Raw data;
  cin >> data.account;
  cin >> data.pass_md5;
  cin >> data.nickname;
  // std::thread go([ ata ]() {
  // fake
  std::lock_guard<std::mutex> lck(send_mutex);
  data.send_data(conn);
  // });
}

void log_in(TCP conn) {
  LoginIn::Raw data;
  cin >> data.account;
  cin >> data.pass_md5;
  global.account = data.account;
  global.passwd = data.pass_md5;
  std::lock_guard<std::mutex> lck(send_mutex);
  data.send_data(conn);
}

void sendmsg(TCP conn) {
  LOG(global.lookup.size());
  SendMessage::Raw data;
  cin >> data.receiver;
  data.timestamp = time(NULL);
  COPY(data.sender, global.account);
  cin >> data.message;
  std::lock_guard<std::mutex> lck(send_mutex);
  auto iter = global.lookup.find(data.receiver);
  if (iter != global.lookup.end()) {
    // P2P mode
    data.send_data(iter->second);
  } else {
    data.send_data(conn);
  }
}

void p2p_request(TCP conn) {
  P2PRequest::Raw data;
  COPY(data.sender, global.account);
  cin >> data.receiver;
  // data.listener_ip
  data.listener_port = global.self_server.getsockname().get_port();
  std::lock_guard<std::mutex> lck(send_mutex);
  data.send_data(conn);
}

void file_send() {
  FileSendRequest::Raw data;
  cin >> data.receiver;
  auto iter = global.lookup.find(data.receiver);
  if (iter == global.lookup.end()) {
    cerr << "no p2p connection set up" << endl;
  }
  cin >> data.file_path;
  LOG(data.file_path);
  COPY(data.sender, global.account);
  data.file_length = FileUDP::len(data.file_path);
  LOG(data.file_length);
  data.send_data(iter->second);
}

void file_accept() {
  extern FileSendAccept::Raw data;
  string local_file_path;
  cin >> local_file_path;
  LOG(local_file_path);
  LOG(data.file_path);
  LOG(data.file_length);
  LOG(data.sender);
  data.udp_port = FileUDP::open_receive_port(local_file_path, data.file_length);
  auto iter = global.lookup.find(data.sender);
  if (iter == global.lookup.end()) {
    cerr << "p2p not set";
  }
  data.send_data(iter->second);
}

int main() {
  cout << "client" << endl;
  TCP client;
  client.socket();
  client.connect(SERVER_IP, SERVER_PORT);
  global.client = client;
  // go.detach();
  TCP self_server;
  self_server.socket();
  self_server.bind(0);
  self_server.listen();
  global.self_server = self_server;
  std::thread t(
      [](TCP self_server, TCP client) {
        global.engine.set_server(self_server);
        global.engine.insert(client);
        global.engine.run();
      },
      self_server, client);
  string req;
  while (cin >> req) {
    // wait for event
    if (false) {
    } else if (req == "up2") {
      FileSendRequest::Raw data{};
      strcpy(data.receiver, "mike");
      auto iter = global.lookup.find(data.receiver);
      if (iter == global.lookup.end()) {
        cerr << "no p2p connection set up" << endl;
      }
      strcpy(data.file_path, "/home/mike/intel.tgz");
      LOG(data.file_path);
      COPY(data.sender, global.account);
      data.file_length = FileUDP::len(data.file_path);
      LOG(data.file_length);
      data.send_data(iter->second);

    } else if (req == "down2") {
      extern FileSendAccept::Raw data;
      string local_file_path;
      local_file_path = "/home/mike/ggi.tgz";
      LOG(local_file_path);
      LOG(data.file_path);
      LOG(data.file_length);
      LOG(data.sender);
      data.udp_port =
          FileUDP::open_receive_port(local_file_path, data.file_length);
      auto iter = global.lookup.find(data.sender);
      if (iter == global.lookup.end()) {
        cerr << "p2p not set";
      }
      data.send_data(iter->second);
    } else if (req == "up") {
      LoginIn::Raw data;
      strcpy(data.account, "fluorine");
      strcpy(data.pass_md5, "wang");
      global.account = data.account;
      global.passwd = data.pass_md5;
      {
        std::lock_guard<std::mutex> lck(send_mutex);
        data.send_data(client);
      }
      {
        P2PRequest::Raw data;
        COPY(data.sender, global.account);
        strcpy(data.receiver, "mike");
        // data.listener_ip
        data.listener_port = global.self_server.getsockname().get_port();
        std::lock_guard<std::mutex> lck(send_mutex);
        data.send_data(client);
      }
    } else if (req == "down") {
      LoginIn::Raw data;
      strcpy(data.account, "mike");
      strcpy(data.pass_md5, "wang");
      global.account = data.account;
      global.passwd = data.pass_md5;
      std::lock_guard<std::mutex> lck(send_mutex);
      data.send_data(client);
    } else if (req == "register") {
      // onEvent click
      // easily parallel
      registion(client);
    } else if (req == "login") {
      // onEvent click
      // easily parallel
      log_in(client);
    } else if (req == "send") {
      sendmsg(client);
    } else if (req == "p2p") {
      p2p_request(client);
    } else if (req == "filesend") {
      file_send();
    } else if (req == "fileAC") {
      file_accept();
    } else {
      cout << "ignored" << endl;
    }
  }
  // while (true) {
  //   cin >> str;
  //   int n = strlen(str);
  //   client.writen(str, n);
  //   int ref_n = client.read(ref_str, BUFFER_SIZE);
  //   if (ref_n == 0) {
  //     cerr << "[ERROR] server is gone " << endl;
  //     exit(-1);
  //   } else {
  //     cout << ref_n << "***" << ref_str << endl;
  //   }
  // }
}
