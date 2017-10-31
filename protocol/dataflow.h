#ifndef DOG_DATAFLOW_H_
#define DOG_DATAFLOW_H_

#include "../include/common.h"
#include "../include/dog_socket.h"
#include <cassert>
#include <memory>

enum class DataFlowType {
  Type_Registion,
  Type_OpStatus,
};

class RawData {
public:
  virtual void read_data(TCP conn) = 0;
  // static void send_data(TCP conn) = 0;
  virtual void action(TCP conn) = 0;
  static std::unique_ptr<RawData> get_type(TCP &conn);

protected:
  static DataFlowType TYPE_ID;
};

#define DEFINE_CLASS(name)
// client -> server
// make AM great again
class Registion : public RawData {
public:
  virtual void read_data(TCP conn) override { conn.readn(&raw, sizeof(raw)); }
  virtual void action(TCP conn) override;
  struct Raw {
    void send_data(TCP conn) {
      cerr << "sending regi" << endl;
      constexpr auto id = DataFlowType::Type_Registion;
      conn.writen(&id, sizeof(id));
      conn.writen(this, sizeof(raw));
    }
    char account[32];
    char nickname[32];
    char pass_md5[32];
  } raw;
};

// server -> client
// make AM great again
class OpStatus : public RawData {
public:
  virtual void read_data(TCP conn) override {
    // read from socket
    conn.readn(&raw, sizeof(raw));
  }
  virtual void action(TCP conn) override;
  struct Raw {
    void send_data(TCP conn) {
      cerr << "sending opt" << endl;
      const auto id = DataFlowType::Type_OpStatus;
      conn.writen(&id, sizeof(id));
      conn.writen(this, sizeof(raw));
    }
    int status;
    char message[32];
  } raw;
};

#endif // DOG_DATAFLOW_H_