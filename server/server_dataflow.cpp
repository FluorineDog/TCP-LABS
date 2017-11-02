#define NEWCASE(type)                                                          \
  case DataFlowType::Type_##type:                                              \
    data = std::make_unique<type>();                                           \
    break

#define FAKECASE(type)                                                         \
  case DataFlowType::Type_##type:                                              \
    cerr << "receive fake symbol " << #type << endl;                           \
    assert(s != DataFlowType::Type_##type)

#define SERVERCASE(type) NEWCASE(type)
#define CLIENTCASE(type) FAKECASE(type)

#include "../protocol/gen_dataflow.h"

extern sqlite::connection sql;

// #define SQL_EXECUTE(cmd, instruction)                                           \
  // static sqlite::execute cmd(sql, instruction);

// in server
void Registion::action(TCP conn) {
  // in server
  // check if is in database
  cerr << "get account:" << raw.account << endl;
  cerr << "get passwd:" << raw.pass_md5 << endl;
  cerr << "get nickname:" << raw.nickname << endl;
  // seems to have put it into db
  static sqlite::execute cmd(sql, "INSERT INTO accounts VALUES(?,?,?,?)");
  cmd.clear();
  try {
    cmd % raw.account % raw.pass_md5 % raw.nickname % sqlite::nil;
    cmd();
  } catch (std::exception &e) {
    OpStatus::Raw reply{-1, ""};
    // strcpy_n(reply.message, )
    strncpy(reply.message, e.what(), 32);
    reply.send_data(conn);
    return;
  }
  OpStatus::Raw{0, "succceed"}.send_data(conn);
}

// extern std::map<int, string> conn_to_account;
extern std::map<string, int> lookup;

void send_offline_message(TCP conn, const string &receiver);

void LoginIn::action(TCP conn) {
  LOG((int)conn);
  // in server
  // check if is in atabase
  cerr << "get request account:" << raw.account << endl;
  cerr << "get passwd:" << raw.pass_md5 << endl;
  // seems to have put it into db
  if (lookup.find(raw.account) != lookup.end()) {
    OpStatus::Raw data{-1, "Logged in elsewhwere"};
    data.send_data(conn);
    return;
  }

  static sqlite::query q(
      sql, "SELECT password, salt, nickname from accounts where account=?");
  q.clear();
  q % raw.account;
  shared_ptr<sqlite::result> result = q.get_result();
  cerr << "$" << result->get_row_count() << "$" << endl;
  if (result->next_row()) {
    string true_pass = result->get_string(0);
    string salt = result->get_string(1);
    cerr << "done" << true_pass;
    if (true_pass == raw.pass_md5) {
      string nickname = result->get_string(2);
      LoginReply::Raw data{0, "login succeed"};
      // conn_to_account[conn] = raw.account;
      lookup[raw.account] = conn;
      COPY(data.nickname, nickname);
      data.send_data(conn);
      send_offline_message(conn, raw.account);
      return;
    }
  }
  OpStatus::Raw data{-1, "wrong account or password"};
  data.send_data(conn);
  return;
}

void SendMessage::action(TCP conn) {
  auto iter = lookup.find(raw.receiver);
  if (iter != lookup.end()) {
    // directly send
    // assert(false);
    cerr << iter->second;
    TCP receiver_conn(iter->second);
    raw.send_data(receiver_conn);
  } else {
    //
    static sqlite::execute cmd(sql, "INSERT INTO message VALUES(NULL,?,?,?,?)");
    cmd.clear();
    cmd % raw.sender % raw.receiver % raw.timestamp % raw.message;
    cmd();
  }
}

void send_offline_message(TCP conn, const string &receiver) {
  static sqlite::query q(sql, "SELECT rowid, receiver, timestamp, message"
                              " FROM message WHERE receiver=?");
  q.clear();
  q % receiver;
  auto result = q.get_result();

  SendMessage::Raw data;
  COPY(data.receiver, receiver);
  while (result->next_row()) {
    COPY(data.sender, result->get_string(1));
    data.timestamp = result->get_int64(2);
    COPY(data.message, result->get_string(3));
    data.send_data(conn);
  }
  static sqlite::execute cmd(sql, "DELETE FROM message WHERE receiver=?");
  cmd.clear();
  cmd % receiver;
  cmd();
}

void P2PRequest::action(TCP conn) {
  auto iter = lookup.find(raw.receiver);
  if (iter != lookup.end()) {
    auto addr = conn.getpeername();
    raw.listener_ip = ::ntohl(addr.sin_addr.s_addr);
    LOG(addr);
    LOG(raw.listener_port);
    raw.send_data(iter->second);
  } else {
    OpStatus::Raw{-1, "User offline"}.send_data(conn);
  }
}
