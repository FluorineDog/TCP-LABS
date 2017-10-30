#include <iostream>
using namespace std;
// #include "str_const.h"
#include <gtest/gtest.h>
// #include <openssl/crypto.h>
#include
#include <sqlite3.h>
// struct Factory {
//   Factory(const char *str) : str(str) {}
//   ~Factory() {}

// private:
//   char *str;
// };

TEST(libcrypto, base) {}

TEST(sqlite, base) {
  sqlite3 *db;
  char *errmsg;
  sqlite3_open("test.db", &db);
  sqlite3_exec(db, "select * from account_info",
               [](void *, int n, char **strs, char **names) {
                 cerr << n;
                 for (int i = 0; i < n; ++i) {
                   cerr << "|" << strs[i];
                 }
                 cerr << endl;
                 return 0;
               },
               nullptr, &errmsg);
  EXPECT_EQ(errmsg, nullptr);
  sqlite3_prepare_v2(db, "select * from account_info", ;
  // cerr << errmsg;
}