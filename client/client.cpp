#include "../include/common.h"
#include "../include/dog_socket.h"
#include "../protocol/dataflow.h"
using std::cin;
using std::cout;
using std::cerr;
using std::endl;

void registion(TCP conn) {
  Registion::Raw data;
  cin >> data.account;
  cin >> data.nickname;
  cin >> data.pass_md5;
  // std::thread go([ ata ]() {
  // fake
  data.send_data(conn);
  // });
}

void listener(TCP conn) {
  while (true) {
    auto data = RawData::get_type(conn);
    if(data == nullptr){
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
  while (true) {
    string req;
    // wait for event
    cin >> req;
    if (req == "register") {
      // onEvent click
      // easily parallel
      registion(client);
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
