#ifndef __VECTOR__
#define __VECTOR__

#include "_common.hpp"
#include <cstddef>
#include <memory>

namespace mstl {

template <typename T, typename Alloc = std::allocator<T>> class vector {
public:
  using value_type = T;
  using allocator_type = Alloc;
  using size_type = std::size_t;
  using diff_type = std::ptrdiff_t;
  using ptr = T *;
  using const_ptr = T const *;
  using ref = T &;
  using const_ref = T const &;
  using it = T *;
  using const_it = T const *;
  using rev_it = std::reverse_iterator<T *>;
  using const_rev_it = std::reverse_iterator<T const *>;

private:
  T *m_data;
  std::size_t m_size;
  std::size_t m_capacity;
  [[no_unique_address]] Alloc m_allocator;

public:
    

  _LIBPENGCXX_DEFINE_COMPARISON(vector);
};
} // namespace mstl

#endif // !__VECTOR__