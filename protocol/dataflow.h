#ifndef DOG_DATAFLOW_H_
#define DOG_DATAFLOW_H_

#include "../include/common.h"
#include "../include/dog_socket.h"
#include <cassert>
#include <memory>

enum class DataFlowType {
  Type_Registion,
  Type_OpStatus,
  Type_LoginIn,
  Type_LoginReply,
  Type_SendMessage,
  Type_P2PRequest,
  Type_P2PLogin,
};

class RawData {
public:
  virtual int read_data(TCP conn) = 0;
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
  virtual int read_data(TCP conn) override {
    return conn.readn(&raw, sizeof(raw));
  }
  virtual void action(TCP conn) override;
  struct Raw {
    void send_data(TCP conn) {
      constexpr auto id = DataFlowType::Type_Registion;
      conn.writen(&id, sizeof(id));
      conn.writen(this, sizeof(raw));
    }
    char account[32];
    char pass_md5[32];
    char nickname[32];
  } raw;
};

class LoginIn : public RawData {
public:
  virtual int read_data(TCP conn) override {
    return conn.readn(&raw, sizeof(raw));
  }
  virtual void action(TCP conn) override;
  struct Raw {
    void send_data(TCP conn) {
      constexpr auto id = DataFlowType::Type_LoginIn;
      conn.writen(&id, sizeof(id));
      conn.writen(this, sizeof(raw));
    }
    char account[32];
    char pass_md5[32];
    // char nickname[32];
  } raw;
};

// server -> client
// make AM great again
class OpStatus : public RawData {
public:
  virtual int read_data(TCP conn) override {
    // read from socket
    return conn.readn(&raw, sizeof(raw));
  }
  virtual void action(TCP conn) override;
  struct Raw {
    void send_data(TCP conn) {
      const auto id = DataFlowType::Type_OpStatus;
      conn.writen(&id, sizeof(id));
      conn.writen(this, sizeof(raw));
    }
    int status;
    char message[32];
  } raw;
};

class LoginReply : public RawData {
public:
  virtual int read_data(TCP conn) override {
    // read from socket
    return conn.readn(&raw, sizeof(raw));
  }
  virtual void action(TCP conn) override;
  struct Raw {
    void send_data(TCP conn) {
      const auto id = DataFlowType::Type_LoginReply;
      conn.writen(&id, sizeof(id));
      conn.writen(this, sizeof(raw));
    }
    int status;
    char message[32];
    char nickname[32];
  } raw;
};

class SendMessage : public RawData {
public:
  virtual int read_data(TCP conn) override {
    // read from socket
    return conn.readn(&raw, sizeof(raw));
  }
  virtual void action(TCP conn) override;
  struct Raw {
    void send_data(TCP conn) {
      const auto id = DataFlowType::Type_SendMessage;
      conn.writen(&id, sizeof(id));
      conn.writen(this, sizeof(raw));
    }
    char sender[32];
    char receiver[32];
    time_t timestamp;
    char message[512];
  } raw;
};

class P2PRequest : public RawData {
public:
  virtual int read_data(TCP conn) override {
    // read from socket
    return conn.readn(&raw, sizeof(raw));
  }
  virtual void action(TCP conn) override;
  struct Raw {
    void send_data(TCP conn) {
      const auto id = DataFlowType::Type_P2PRequest;
      conn.writen(&id, sizeof(id));
      conn.writen(this, sizeof(raw));
    }
    char sender[32];
    char receiver[32];
    unsigned listener_ip;
    int listener_port;
  } raw;
};

class P2PLogin : public RawData {
public:
  virtual int read_data(TCP conn) override {
    // read from socket
    return conn.readn(&raw, sizeof(raw));
  }
  virtual void action(TCP conn) override;
  struct Raw {
    void send_data(TCP conn) {
      const auto id = DataFlowType::Type_P2PLogin;
      conn.writen(&id, sizeof(id));
      conn.writen(this, sizeof(raw));
    }
    char peer[32];
  } raw;
};


#endif // DOG_DATAFLOW_H_