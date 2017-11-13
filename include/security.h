#ifndef DOG_SECURITY_H_
#define DOG_SECURITY_H_

#include <cassert>
#include <cstddef>
#include <nmmintrin.h>
inline unsigned int naive_hash(const void *data, int size) {
  // work only for Plain Old Data (POD)
  // stupid but efficient for random data
  // unsafe for attack, but security is NOT required
  // using T = unsigned long long;
  using T = char;
  static_assert(sizeof(T) == 1, "fake compiler");
  assert((size_t)data % 1 == 0);
  assert(size % sizeof(T) == 0);
  size /= sizeof(T);
  unsigned crc = 0;
  T *data_ = (T *)data;
  for (int i = 0; i < size; ++i) {
    crc = _mm_crc32_u64(crc, data_[i]);
  }
  return crc;
}

#endif // DOG_SECURITY_H_
