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
  conn.readn((char *)&s, sizeof(s));
  std::unique_ptr<RawData> data;
  switch (s) {
    // case DataFlowType::Registion:
    // data = std::make_unique<Registion>(new Registion);
    // }
    SERVERCASE(Registion);
    CLIENTCASE(OpStatus);
  }
  return data;
}
#undef SERVERCASE
#undef CLIENTCASE
#undef NEWCASE
#undef FAKECASE

// in server
void Registion::action(TCP conn) {
  // in server
  // check if is in database
  cerr << "get account:" << raw.account << endl;
  cerr << "get nikename:" << raw.nickname << endl;
  cerr << "get passwd:" << raw.pass_md5 << endl;
  // seems to have put it into db
  OpStatus::Raw data{0, "update succeed"};
  data.send_data(conn);
}

