#include "../protocol/dataflow.h"
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

std::unique_ptr<RawData> RawData::get_type(TCP &conn) {
  DataFlowType s;
  cerr << "begin read typeid" << endl;
  int status = conn.readn(&s, sizeof(s));
  if (status == 0) {
    return nullptr;
  }
  std::unique_ptr<RawData> data;
  switch (s) {
    // case DataFlowType::Registion:
    // data = std::make_unique<Registion>(new Registion);
    // }
    SERVERCASE(Registion);
    CLIENTCASE(OpStatus);
    SERVERCASE(LoginIn);
    CLIENTCASE(LoginReply);
  default:
    cerr << "unknown typeid" << endl;
    exit(-1);
  }
  return data;
}

#undef SERVERCASE
#undef CLIENTCASE
#undef NEWCASE
#undef FAKECASE

extern sqlite::connection sql;

// #define SQL_EXECUTE(cmd, instruction)                                           \
  // static sqlite::execute cmd(sql, instruction);

// in server
void Registion::action(TCP conn) {
  // in server
  // check if is in database
  cerr << "get account:" << raw.account << endl;
  cerr << "get passwd:" << raw.pass_md5 << endl;
  cerr << "get nikename:" << raw.nickname << endl;
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

void LoginIn::action(TCP conn) {
  // in server
  // check if is in atabase
  cerr << "get request account:" << raw.account << endl;
  cerr << "get passwd:" << raw.pass_md5 << endl;
  // seems to have put it into db
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
      strncpy(data.nickname, nickname.c_str(), 32);
      data.send_data(conn);
      return;
    }
  }
  OpStatus::Raw data{-1, "wrong account or password"};
  data.send_data(conn);
  return;
}
