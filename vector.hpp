#ifndef __VECTOR__
#define __VECTOR__

#include "_common.hpp"
#include <cstddef>
#include <initializer_list>
#include <iterator>
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
  std::size_t m_cap;
  [[no_unique_address]] Alloc m_alloc;

public:
  vector() noexcept {
    m_data = nullptr;
    m_size = 0;
    m_cap = 0;
  }

  vector(std::initializer_list<T> ilist, const Alloc &allocator = Alloc())
      : vector(ilist.std::begin(), ilist.std::end(), allocator) {}

  explicit vector(std::size_t n, const Alloc &allocator = Alloc())
      : m_alloc(allocator) {
    m_data = m_alloc.allocate(n);
    m_cap = m_size = n;
    for (size_t i = 0; i < n; i++) {
#if __cpp_lib_constexpr_dynamic_alloc >= 201907L
      std::construct_at(&m_data[i]);
#else
      new (&m_data[i]) T();
#endif
    }
  }

  vector(std::size_t n, const T &init_val, const Alloc &allocator = Alloc())
      : m_alloc(allocator) {
    m_data = m_alloc.allocate(n);
    m_cap = m_size = n;
    for (size_t i = 0; i < n; i++) {
#if __cpp_lib_constexpr_dynamic_alloc >= 201907L
      std::construct_at(&m_data[i], init_val);
#else
      new (&m_data[i]) T();
      m_data[i] = init_val;
#endif
    }
  }

  template <_LIBPENGCXX_REQUIRES_ITERATOR_CATEGORY(std::random_access_iterator,
                                                   It)>
  vector(It first, It last, const Alloc &allocator = Alloc())
      : m_alloc(allocator) {
    size_t n = last - first;
    m_data = m_alloc.allocate(n);
    m_cap = m_size = n;
    for (size_t i = 0; i < n; i++) {
#if __cpp_lib_constexpr_dynamic_alloc >= 201907L
      std::construct_at(&m_data[i], *first);
#else
      new (&m_data[i]) T();
      m_data[i] = *first;
#endif
      ++first;
    }
  }

  T *data() noexcept { return m_data; }
  const T *data() const noexcept { return m_data; }
  const T *cdata() const noexcept { return m_data; }

  T *begin() noexcept { return m_data; }
  const T *begin() const noexcept { return m_data; }
  const T *cbegin() const noexcept { return m_data; }
  T *end() noexcept { return m_data + m_size; }
  const T *end() const noexcept { return m_data + m_size; }
  const T *cend() const noexcept { return m_data + m_size; }

  std::reverse_iterator<T *> rbegin() noexcept {
    return std::make_reverse_iterator(m_data + m_size);
  }

  std::reverse_iterator<T *> rend() noexcept {
    return std::make_reverse_iterator(m_data);
  }

  std::reverse_iterator<const T *> rbegin() const noexcept {
    return std::make_reverse_iterator(m_data + m_size);
  }

  std::reverse_iterator<const T *> rend() const noexcept {
    return std::make_reverse_iterator(m_data);
  }

  std::reverse_iterator<const T *> crbegin() const noexcept {
    return std::make_reverse_iterator(m_data + m_size);
  }

  std::reverse_iterator<const T *> crend() const noexcept {
    return std::make_reverse_iterator(m_data);
  }

  _LIBPENGCXX_DEFINE_COMPARISON(vector);
};

} // namespace mstl

#endif // !__VECTOR__