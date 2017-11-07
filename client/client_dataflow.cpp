#include "../protocol/dataflow.h"

extern struct Global {
  string account;
  string passwd;
  TCP client;
  TCP self_server;
  map<string, TCP> lookup;
  Epoll engine;
} global;

#define NEWCASE(type)                                                          \
  case DataFlowType::Type_##type:                                              \
    data = std::make_unique<type>();                                           \
    break

#define FAKECASE(type)                                                         \
  case DataFlowType::Type_##type:                                              \
    cerr << "receive fake symbol " << #type << endl;                           \
    assert(s != DataFlowType::Type_##type)

#define CLIENTCASE(type) NEWCASE(type)
#define SERVERCASE(type) FAKECASE(type)

#include "../protocol/gen_dataflow.h"

#undef SERVERCASE
#undef CLIENTCASE
#undef NEWCASE
#undef FAKECASE

// in client
void OpStatus::action(TCP conn) {
  // in server
  // check if is in database
  cerr << "get status:" << raw.status << endl;
  cerr << "get info:" << raw.message << endl;
  // cerr << "get nikename:" << raw.nickname << endl;
  // cerr << "get passwd:" << raw.pass_md5 << endl;
  // seems to have put it into db
  // OpStatus::Raw data{0, "update succeed"};
}

void LoginReply::action(TCP conn) {
  cerr << "get status:" << raw.status << endl;
  cerr << "get info:" << raw.message << endl;
  cerr << "get fuck:" << raw.nickname << endl;
}

void SendMessage::action(TCP conn) {
  cerr << "get message " << raw.message << " from " << raw.sender;
}

void P2PRequest::action(TCP) {
  TCP new_conn;
  new_conn.socket();
  // TODO buggy
  new_conn.connect(raw.listener_ip, raw.listener_port);
  global.engine.insert(new_conn);
  global.lookup[raw.sender] = new_conn;
  LOG(raw.listener_port);
  LOG(raw.sender);
  LOG(new_conn);
  P2PLogin::Raw login_data;
  COPY(login_data.peer, global.account);
  // add link to lookup
  login_data.send_data(new_conn);
}

void Epoll::visitor(TCP conn) {
  cerr << "try getting type";
  auto data = RawData::get_type(conn);
  if (data == nullptr) {
    if (conn == global.client) {
      // server is killed
      global.client = -1;
      cerr << "server is down. Please reconnect" << endl;
      this->erase(conn);
      conn.close();
      return;
    } else {
      // client is closed
      cerr << "peer is down" << endl;
      for (auto iter = global.lookup.begin(); iter != global.lookup.end();
           ++iter) {
        if (iter->second == conn) {
          global.lookup.erase(iter);
          break;
        }
      }
      this->erase(conn);
      conn.close();
      return;
    }
  }
  data->read_data(conn);
  data->action(conn);
}

void P2PLogin::action(TCP conn) {
  // add link to lookup
  global.lookup[raw.peer] = conn;
}

FileSendAccept::Raw data;
void FileSendRequest::action(TCP conn) {
  // message box
  LOG(raw.file_path);
  LOG(raw.file_length);
  LOG(raw.sender);
  cout << "FileRequest" << endl;
  COPY(data.sender, raw.sender);
  COPY(data.receiver, raw.receiver);
  COPY(data.file_path, raw.file_path);
  data.file_length = raw.file_length;
  // set up File later
}

#include "../protocol/file_udp.h"
void FileSendAccept::action(TCP conn) {
  // message box
  LOG(raw.file_path);
  LOG(raw.file_length);
  LOG(raw.sender);
  LOG(raw.receiver);
  FileUDP file_udp;
  auto ip = TCP(global.lookup[raw.receiver]).getpeername().get_ip();
  LOG(ip);
  file_udp.send(raw.file_path, ip, raw.udp_port);
}
