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
#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>

byte key[CryptoPP::AES::DEFAULT_KEYLENGTH] = "fluorinedoghere",
     iv[CryptoPP::AES::BLOCKSIZE];

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

    string encryptedMsg;
    string plain = raw.message;
    //
    CryptoPP::AES::Encryption aesEncryption(key,
                                            CryptoPP::AES::DEFAULT_KEYLENGTH);
    CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption,
                                                                iv);

    CryptoPP::StreamTransformationFilter stfEncryptor(
        cbcEncryption, new CryptoPP::StringSink(encryptedMsg));
    stfEncryptor.Put(reinterpret_cast<const unsigned char *>(plain.c_str()),
                     sizeof(plain.size() + 1));
    stfEncryptor.MessageEnd();
    //
    static sqlite::execute cmd(sql, "INSERT INTO message VALUES(NULL,?,?,?,?,?)");
    cmd.clear();
    cmd % raw.sender % raw.receiver % raw.timestamp ;
    cmd % (int)encryptedMsg.size();
    cmd % encryptedMsg;
    cmd();
  }
}

void send_offline_message(TCP conn, const string &receiver) {
  static sqlite::query q(sql, "SELECT rowid, receiver, timestamp, messagesize, message"
                              " FROM message WHERE receiver=?");
  q.clear();
  q % receiver;
  auto result = q.get_result();

  SendMessage::Raw data;

  COPY(data.receiver, receiver);
  while (result->next_row()) {
    COPY(data.sender, result->get_string(1));
    data.timestamp = result->get_int64(2);
    // COPY(data.message, result->get_string(3));
    int messagesize = result->get_int(3);
    string encryptedMsg = result->get_string(4);
    string decryptedMsg;
    CryptoPP::AES::Decryption aesDecryption(key,
                                            CryptoPP::AES::DEFAULT_KEYLENGTH);
    CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption,
                                                                iv);
    CryptoPP::StreamTransformationFilter stfDecryptor(
        cbcDecryption, new CryptoPP::StringSink(decryptedMsg));
    stfDecryptor.Put(reinterpret_cast<const unsigned char *>(encryptedMsg.c_str()),
                     messagesize);
    stfDecryptor.MessageEnd();

    COPY(data.message, decryptedMsg);
    // memcpy(data.message, decrypteMsg.data(), messagesize);
    // for(int i = 0; i < 16; ++i){
    //   cerr << (int)decryptedMsg[i] << " ";
    // }

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
    auto peer_ip = conn.getpeername().get_ip();
    raw.listener_ip = peer_ip;
    LOG(peer_ip);
    LOG(raw.listener_port);
    raw.send_data(iter->second);
  } else {
    OpStatus::Raw{-1, "User offline"}.send_data(conn);
  }
}

void RecoverPassword::action(TCP conn) {
  LOG((int)conn);
  // in server
  // check if is in atabase
  cerr << "get request account:" << raw.account << endl;
  cerr << "get nickname:" << raw.nickname << endl;
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
    string nickname = result->get_string(2);
    string salt = result->get_string(1);
    if (nickname == raw.nickname) {
      string password = result->get_string(0);
      static sqlite::execute exe(
          sql, "UPDATE accounts SET password=?, salt=? where account=?");
      exe.clear();
      exe % raw.new_pass_md5 % salt % raw.account;
      exe();
      OpStatus::Raw data{0, "recover succeed"};
      // conn_to_account[conn] = raw.account;
      data.send_data(conn);
      return;
    }
  }
  OpStatus::Raw data{-1, "wrong account or email"};
  data.send_data(conn);
  return;
}

void FindActiveRequest::action(TCP conn) {
  for (auto p : lookup) {
    auto acc = p.first;
    if (acc == raw.sender) {
      continue;
    }
    FindActiveReply::Raw data;
    data.isActive = true;
    COPY(data.peer, acc);
    data.send_data(conn);
  }
}