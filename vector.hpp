#ifndef __VECTOR__
#define __VECTOR__

#include "_common.hpp"
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <limits>
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

public:
  void clear() noexcept {
    for (size_t i = 0; i < m_size; i++) {
#if __cpp_lib_constexpr_dynamic_alloc >= 201907L
      std::destroy_at(&m_data[i]);
#else
      m_data[i].~T();
#endif
    }
    m_size = 0;
  }

  void resize(size_t n) {
    if (n < m_size) {
      for (size_t i = n; i < m_size; i++) {
#if __cpp_lib_constexpr_dynamic_alloc >= 201907L
        std::destroy_at(&m_data[i]);
#else
        m_data[i].~T();
#endif
      }
    } else if (n > m_size) {
      reserve(n);
      for (size_t i = m_size; i < n; i++) {
#if __cpp_lib_constexpr_dynamic_alloc >= 201907L
        std::construct_at(&m_data[i]);
#else
        new (&m_data[i]) T();
#endif
      }
    }
    m_size = n;
  }

  void resize(size_t n, const T &default_val) {
    if (n < m_size) {
      for (size_t i = n; i < m_size; i++) {
#if __cpp_lib_constexpr_dynamic_alloc >= 201907L
        std::destroy_at(&m_data[i]);
#else
        m_data[i].~T();
#endif
      }
    } else if (n > m_size) {
      reserve(n);
      for (size_t i = m_size; i < n; i++) {
#if __cpp_lib_constexpr_dynamic_alloc >= 201907L
        std::construct_at(&m_data[i], default_val);
#else
        new (&m_data[i]) T();
        m_data[i] = default_val;
#endif
      }
    }
    m_size = n;
  }

  void shrink() noexcept {}

  void reserve(size_t n) {
    if (n <= m_cap)
      return;

    n = std::max(n, m_cap * 2);

    auto old_data = m_data;
    auto old_cap = m_cap;

    if (n == 0) {
      m_data = nullptr;
      m_cap = 0;
    } else {
      m_data = m_alloc.allocate(n);
      m_cap = n;
    }

    if (old_cap != 0) {
      for (size_t i = 0; i < m_size; i++) {
#if __cpp_lib_constexpr_dynamic_alloc >= 201907L
        std::construct_at(&m_data[i], std::move_if_noexcept(old_data[i]));
#else
        new (&m_data[i]) T(std::move_if_noexcept(old_data[i]));
#endif
      }
      for (size_t i = 0; i < m_size; i++) {
#if __cpp_lib_constexpr_dynamic_alloc >= 201907L
        std::destroy_at(&old_data[i]);
#else
        old_data[i].~T();
#endif
      }
      m_alloc.deallocate(old_data, old_cap);
    }
  }

  std::size_t capacity() const noexcept { return m_cap; }
  std::size_t size() const noexcept { return m_size; }
  inline bool empty() const noexcept { return m_size == 0; }
  static constexpr std::size_t max_size() noexcept {
    return std::numeric_limits<std::size_t>::max() / sizeof(T);
  }

public:
  

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