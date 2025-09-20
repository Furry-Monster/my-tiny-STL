#ifndef __RAII__
#define __RAII__

#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <type_traits>
#include <utility>
namespace mstl {

template <typename T> class deleter {
public:
  void operator()(T *p) const { delete p; }
};

template <typename T> class deleter<T[]> {
public:
  void operator()(T *p) const { delete[] p; }
};

template <typename T, typename TDeleter = deleter<T>> class unique_ptr {
private:
  T *m_p;
  [[no_unique_address]] TDeleter m_del;

  template <typename U, typename UDeleter> friend class unique_ptr;

public:
  using element_type = T;
  using pointer = T *;
  using deleter_type = TDeleter;

  unique_ptr(std::nullptr_t = nullptr) noexcept : m_p(nullptr) {}
  explicit unique_ptr(T *p) noexcept : m_p(p) {}

  template <typename U, typename UDeleter,
            typename = std::enable_if_t<std::is_convertible_v<U *, T *>>>
  unique_ptr(unique_ptr<U, UDeleter> &&that) noexcept : m_p(that.m_p) {
    that.m_p = nullptr;
  }

  unique_ptr(unique_ptr const &that) = delete;
  unique_ptr &operator=(unique_ptr const &that) = delete;
  unique_ptr(unique_ptr &&that) noexcept : m_p(that.m_p) { that.m_p = nullptr; }
  unique_ptr &operator=(unique_ptr &&that) noexcept {
    if (this != &that) [[likely]] {
      if (m_p)
        m_del(m_p);
      m_p = that.m_p;
      that.m_p = nullptr;
    }
    return *this;
  }

  ~unique_ptr() noexcept {
    if (m_p)
      m_del(m_p);
  }

  void swap(unique_ptr const &that) noexcept { std::swap(m_p, that.m_p); }

  T *get() const noexcept { return m_p; }
  T *operator->() const noexcept { return m_p; }
  std::add_lvalue_reference_t<T> operator*() const noexcept { return *m_p; }
  TDeleter get_deleter() const noexcept { return m_del; }

  T *release() noexcept {
    T *p = m_p;
    m_p = nullptr;
    return p;
  }

  void reset(T *p = nullptr) noexcept {
    if (m_p)
      m_del(m_p);
    m_p = p;
  }

  explicit operator bool() const noexcept { return m_p != nullptr; }
  bool operator==(unique_ptr const &that) const noexcept {
    return m_p == that.m_p;
  }
  bool operator!=(unique_ptr const &that) const noexcept {
    return m_p != that.m_p;
  }

  bool operator>(unique_ptr const &that) const noexcept {
    return m_p > that.m_p;
  }
  bool operator>=(unique_ptr const &that) const noexcept {
    return m_p >= that.m_p;
  }
  bool operator<(unique_ptr const &that) const noexcept {
    return m_p < that.m_p;
  }
  bool operator<=(unique_ptr const &that) const noexcept {
    return m_p <= that.m_p;
  }
};

template <typename T, typename Deleter>
class unique_ptr<T[], Deleter> : public unique_ptr<T, Deleter> {
public:
  using unique_ptr<T, Deleter>::unique_ptr;

  std::add_lvalue_reference_t<T> operator[](std::size_t idx) {
    return this->get()[idx];
  }
};

template <typename T, typename... Args,
          typename = std::enable_if_t<!std::is_array_v<T>>>
unique_ptr<T> make_unique(Args &&...args) {
  return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T, typename = std::enable_if_t<!std::is_array_v<T>>>
unique_ptr<T> make_unique() {
  return unique_ptr<T>(new T);
}

template <typename T, typename = std::enable_if_t<std::is_array_v<T>>>
unique_ptr<T> make_unique(size_t len, T init_val) {
  auto ret = unique_ptr<T>(new std::remove_extent_t<T>[len]());
  std::fill(ret.get(), ret.get() + (len * sizeof(std::remove_extent_t<T>)),
            init_val);
  return ret;
}

template <typename T, typename = std::enable_if_t<std::is_array_v<T>>>
unique_ptr<T> make_unique(size_t len) {
  static_assert(
      !std::is_const_v<std::remove_extent_t<T>>,
      "Cannot create unique_ptr to const array without initialization.");
  return unique_ptr<T>(new std::remove_extent_t<T>[len]);
}

} // namespace mstl

#endif // !__RAII__
