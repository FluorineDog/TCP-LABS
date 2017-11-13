#include <iostream>
using namespace std;
// #include "str_const.h"
#include <gtest/gtest.h>
// #include <openssl/crypto.h>
#include <sqlite3.h>
// struct Factory {
//   Factory(const char *str) : str(str) {}
//   ~Factory() {}

// private:
//   char *str;
// };

TEST(libcrypto, base) {}

TEST(vector_array, base){
  vector<char[1000]> fake(2);
  memset(fake[0], 23, 2000);
  EXPECT_EQ(23, fake[1][233]);
}
// TEST(sqlite, base) {
//   sqlite3 *db;
//   char *errmsg;
//   const char *tail;
//   sqlite3_open("test.db", &db);
//   sqlite3_exec(db, "select * from account_info",
//                [](void *, int n, char **strs, char **names) {
//                  cerr << n;
//                  for (int i = 0; i < n; ++i) {
//                    cerr << "|" << strs[i];
//                  }
//                  cerr << endl;
//                  return 0;
//                },
//                nullptr, &errmsg);

//   // EXPECT_EQ(4, strlen(errmsg));
//   EXPECT_EQ(nullptr, errmsg);
//   sqlite3_stmt *stmt;
//   sqlite3_prepare_v2(db, "select * from account_info", -1, &stmt, &tail);
//   // sqlite3_bind_
//   sqlite3_step(stmt);
//   // cerr << "$" << tail << "$";
//   EXPECT_EQ(0, tail[0]);
//   sqlite3_finalize(stmt);
//   // cerr << errmsg;
// }
#include <cstddef>
#include <nmmintrin.h>

TEST(crc, test1) {
  union {
    unsigned long long ull;
    unsigned ui[2];
  };
  ull = 0x0123'4567'89ab'cdef;
  unsigned long long crc1 = 0;
  unsigned crc2 = 0;
  crc1 = _mm_crc32_u64(crc1, ull);
  crc2 = _mm_crc32_u32(crc2, ui[0]);
  crc2 = _mm_crc32_u32(crc2, ui[1]);
  EXPECT_EQ(crc1, crc2);
  // unsigned int crc = 1;
  // unsigned int input = 50000;
  // unsigned int res = _mm_crc32_u32(crc, input);
  // EXPECT_EQ(971731851, res);
}
