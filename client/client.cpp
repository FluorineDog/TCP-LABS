#include "../include/common.h"
#include "../include/dog_socket.h"
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
int main() {
  int fake;
  cout << "client" << endl;
  TCP client;
  client.socket();
  client.connect(SERVER_IP, SERVER_PORT);
  char str[BUFFER_SIZE];
  char ref_str[BUFFER_SIZE];
  while (true) {
    cin >> str;
    int n = strlen(str);
    client.writen(str, n);
    int ref_n = client.read(ref_str, BUFFER_SIZE);
    
    if (ref_n == 0) {
      cerr << "[ERROR] server is gone " << endl;
      exit(-1);
    } else {
      cout <<  ref_n << "***" << ref_str << endl;
    }
  }
}
