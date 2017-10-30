#ifndef DOG_STR_CONST_H_
#define DOG_STR_CONST_H_

#include <algorithm>
#include <exception>

class str_const {
public:
  template <std::size_t N>
  constexpr str_const(const char (&a)[N])
      : // ctor
        p_(a),
        sz_(N - 1) {}
  constexpr char operator[](std::size_t n) { // []
    return n < sz_ ? p_[n] : throw std::out_of_range("");
  }
  constexpr std::size_t size() { return sz_; } // size()
  constexpr bool operator==(const str_const &v) {
    if (sz_ != v.sz_)
      return false;
    for (int i = 0; i < sz_; ++i) {
      if (p_[sz_] != v.p_[sz_]) {
        return false;
      }
    }
    return true;
  }

private:
  const char *const p_;
  const std::size_t sz_;
};
#endif // DOG_STR_CONST_H_