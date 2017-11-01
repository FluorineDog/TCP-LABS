#include "../protocol/dataflow.h"
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