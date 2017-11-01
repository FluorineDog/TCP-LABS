#include "../protocol/dataflow.h"

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