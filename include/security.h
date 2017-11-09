#ifndef DOG_SECURITY_H_
#define DOG_SECURITY_H_

#include <cassert>
#include <cstddef>
#include <nmmintrin.h>
inline unsigned int naive_hash(const void *data, int size) {
  // work only for Plain Old Data (POD)
  // stupid but efficient for random data
  // unsafe for attack, but security is NOT required
  static_assert(sizeof(unsigned long long) == 8, "fake compiler");
  assert((size_t)data % 8 == 0);
  assert(size % 8 == 0);
  size /= 8;
  unsigned long long crc = 0;
  unsigned long long *data_ = (unsigned long long *)data;
  for (int i = 0; i < size; ++i) {
    crc = _mm_crc32_u64(crc, data_[i]);
  }
  return (unsigned)crc;
}

#endif // DOG_SECURITY_H_
