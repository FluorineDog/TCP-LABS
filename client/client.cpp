#include "../include/common.h"
#include "../include/dog_socket.h"
#include "../protocol/dataflow.h"
#include <mutex>
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
struct {
  string account;
  string passwd;
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
  SendMessage::Raw data;
  cin >> data.receiver;
  data.timestamp = time(NULL);
  COPY(data.sender, global.account);
  cin >> data.message;
  std::lock_guard<std::mutex> lck(send_mutex);
  data.send_data(conn);
}

void listener(TCP conn) {
  while (true) {
    auto data = RawData::get_type(conn);
    if (data == nullptr) {
      cerr << "server is down" << endl;
      exit(-1);
    }
    data->read_data(conn);
    data->action(conn);
  }
}

int main() {
  cout << "client" << endl;
  TCP client;
  client.socket();
  client.connect(SERVER_IP, SERVER_PORT);
  char str[BUFFER_SIZE];
  char ref_str[BUFFER_SIZE];
  std::thread go(listener, client);
  go.detach();
  string req;
  cout << ">";
  while (cin >> req) {
    // wait for event
    if (false) {
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
    } else {
      cout << "ignored" << endl;
    }
    cout << ">";
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
