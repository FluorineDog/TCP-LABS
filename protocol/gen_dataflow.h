// alternative for C++ 20 feature
// no header protection by design
#include "../protocol/dataflow.h"
#define BOTHCASE(name) NEWCASE(name)

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
    BOTHCASE(SendMessage);
    BOTHCASE(P2PRequest);
    CLIENTCASE(P2PLogin);
    CLIENTCASE(FileSendRequest);
    CLIENTCASE(FileSendAccept);
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
