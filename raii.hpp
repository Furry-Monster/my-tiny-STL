#ifndef __RAII__
#define __RAII__

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdio>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace mstl {

// unique ptr:

template <typename T> class deleter {
public:
  void operator()(T *p) const { delete p; }
};

template <typename T> class deleter<T[]> {
public:
  void operator()(T *p) const { delete[] p; }
};

template <typename T, typename TDeleter = deleter<T>> class unique_ptr {
public:
  using element_type = T;
  using pointer = T *;
  using deleter_type = TDeleter;

private:
  T *m_p;
  [[no_unique_address]] TDeleter m_del;

  template <typename U, typename UDeleter> friend class unique_ptr;

public:
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

// shared ptr:

struct sp_cnt {
  std::atomic<long> m_ref_cnt;

  sp_cnt() noexcept : m_ref_cnt(1) {}

  sp_cnt(sp_cnt &&) = delete;
  sp_cnt(const sp_cnt &) = delete;
  sp_cnt &operator=(sp_cnt &&) = delete;
  sp_cnt &operator=(const sp_cnt &) = delete;

  void inc_ref() noexcept { m_ref_cnt.fetch_add(1, std::memory_order_relaxed); }

  void dec_ref() noexcept {
    if (m_ref_cnt.fetch_sub(1, std::memory_order_relaxed) == 1) {
      delete this;
    }
  }

  long cnt_ref() const noexcept {
    return m_ref_cnt.load(std::memory_order_relaxed);
  }

  virtual ~sp_cnt() = default;
};

template <typename T, typename TDeleter> struct sp_cnt_impl final : sp_cnt {
  T *m_ptr;
  [[no_unique_address]] TDeleter m_deleter;

  explicit sp_cnt_impl(T *ptr) noexcept : m_ptr(ptr) {}

  explicit sp_cnt_impl(T *ptr, TDeleter deleter) noexcept
      : m_ptr(ptr), m_deleter(std::move(deleter)) {}

  ~sp_cnt_impl() noexcept override { m_deleter(m_ptr); }
};

template <typename T, typename TDeleter>
struct sp_cnt_impl_fused final : sp_cnt {
  T *m_ptr;
  void *m_mem;
  [[no_unique_address]] TDeleter m_deleter;

  explicit sp_cnt_impl_fused(T *ptr, void *mem, TDeleter deleter) noexcept
      : m_ptr(ptr), m_mem(mem), m_deleter(deleter) {}

  ~sp_cnt_impl_fused() noexcept { m_deleter(m_ptr); }

  void operator delete(void *mem) {
#if __cpp_aligned_new
    // at least cpp17 should support this
    ::operator delete(mem, std::align_val_t(std::max(
                               alignof(T), alignof(sp_cnt_impl_fused))));
#else
    ::operator delete(mem);
#endif
  }
};

template <typename T> class shared_ptr {
private:
  T *m_ptr;
  sp_cnt *m_owner;

  template <typename> friend class shared_ptr;

  explicit shared_ptr(T *ptr, sp_cnt *owner) noexcept
      : m_ptr(ptr), m_owner(owner) {}

public:
  using element_type = T;
  using pointer = T *;

  shared_ptr(std::nullptr_t = nullptr) noexcept : m_owner(nullptr) {}

  template <typename U,
            std::enable_if_t<std::is_convertible_v<U *, T *>, int> = 0>
  explicit shared_ptr(U *ptr)
      : m_ptr(ptr), m_owner(new sp_cnt_impl<U, deleter<U>>(ptr)) {
    setup_enable_shared_from_this(m_ptr, m_owner);
  }

  template <typename U, typename UDeleter,
            std::enable_if_t<std::is_convertible_v<U *, T *>, int> = 0>
  explicit shared_ptr(U *ptr, UDeleter deleter)
      : m_ptr(ptr),
        m_owner(new sp_cnt_impl<U, UDeleter>(ptr, std::move(deleter))) {
    setup_enable_shared_from_this(m_ptr, m_owner);
  }

  template <class U, class UDeleter,
            std::enable_if_t<std::is_convertible_v<U *, T *>, int> = 0>
  explicit shared_ptr(unique_ptr<U, UDeleter> &&ptr)
      : shared_ptr(ptr.release(), ptr.get_deleter()) {}

  template <typename U>
  inline friend shared_ptr<U> make_shared_fused(U *ptr, sp_cnt *owner) noexcept;

  shared_ptr(const shared_ptr &that) noexcept
      : m_ptr(that.m_ptr), m_owner(that.m_owner) {
    if (m_owner)
      m_owner->inc_ref();
  }

  template <typename U,
            std::enable_if_t<std::is_convertible_v<U *, T *>, int> = 0>
  shared_ptr(const shared_ptr<U> &that) noexcept
      : m_ptr(that.m_ptr), m_owner(that.m_owner) {
    if (m_owner)
      m_owner->inc_ref();
  }

  shared_ptr(shared_ptr &&that) noexcept
      : m_ptr(that.m_ptr), m_owner(that.m_owner) {
    that.m_ptr = nullptr;
    that.m_owner = nullptr;
  }

  template <typename U,
            std::enable_if_t<std::is_convertible_v<U *, T *>, int> = 0>
  shared_ptr(shared_ptr<U> &&that) : m_ptr(that.m_ptr), m_owner(that.m_owner) {
    that.m_ptr = nullptr;
    that.m_owner = nullptr;
  }

  template <typename U>
  shared_ptr(const shared_ptr<U> &that, T *ptr) noexcept
      : m_ptr(ptr), m_owner(that.m_owner) {
    if (m_owner)
      m_owner->inc_ref();
  }

  template <typename U>
  shared_ptr(shared_ptr<U> &&that, T *ptr) noexcept
      : m_ptr(ptr), m_owner(that.m_owner) {
    that.m_ptr = nullptr;
    that.m_owner = nullptr;
  }

  shared_ptr &operator=(const shared_ptr &that) noexcept {
    if (this == &that) {
      return *this;
    }
    if (m_owner) {
      m_owner->dec_ref();
    }
    m_ptr = that.m_ptr;
    m_owner = that.m_owner;
    if (m_owner) {
      m_owner->inc_ref();
    }
    return *this;
  }

  shared_ptr &operator=(shared_ptr &&that) noexcept {
    if (this == &that) {
      return *this;
    }
    if (m_owner) {
      m_owner->dec_ref();
    }
    m_ptr = that.m_ptr;
    m_owner = that.m_owner;
    that.m_ptr = nullptr;
    that.m_owner = nullptr;
    return *this;
  }

  template <class U, std::enable_if_t<std::is_convertible_v<U *, T *>, int> = 0>
  shared_ptr &operator=(const shared_ptr<U> &that) noexcept {
    if (this == &that) {
      return *this;
    }
    if (m_owner) {
      m_owner->dec_ref();
    }
    m_ptr = that.m_ptr;
    m_owner = that.m_owner;
    if (m_owner) {
      m_owner->inc_ref();
    }
    return *this;
  }

  template <class U, std::enable_if_t<std::is_convertible_v<U *, T *>, int> = 0>
  shared_ptr &operator=(shared_ptr<U> &&that) noexcept {
    if (this == &that) {
      return *this;
    }
    if (m_owner) {
      m_owner->dec_ref();
    }
    m_ptr = that.m_ptr;
    m_owner = that.m_owner;
    that.m_ptr = nullptr;
    that.m_owner = nullptr;
    return *this;
  }

  void reset() noexcept {
    if (m_owner) {
      m_owner->dec_ref();
    }
    m_owner = nullptr;
    m_ptr = nullptr;
  }

  template <class U> void reset(U *ptr) {
    if (m_owner) {
      m_owner->dec_ref();
    }
    m_ptr = nullptr;
    m_owner = nullptr;
    m_ptr = ptr;
    m_owner = new sp_cnt_impl<U, deleter<U>>(ptr);
    _S_setupEnableSharedFromThis(m_ptr, m_owner);
  }

  template <class U, class UDeleter> void reset(U *ptr, UDeleter deleter) {
    if (m_owner) {
      m_owner->dec_ref();
    }
    m_ptr = nullptr;
    m_owner = nullptr;
    m_ptr = ptr;
    m_owner = new sp_cnt_impl<U, UDeleter>(ptr, std::move(deleter));
    _S_setupEnableSharedFromThis(m_ptr, m_owner);
  }

  ~shared_ptr() noexcept {
    if (m_owner) {
      m_owner->dec_ref();
    }
  }

  long use_count() noexcept { return m_owner ? m_owner->cnt_ref() : 0; }

  bool unique() noexcept { return m_owner ? m_owner->cnt_ref() == 1 : true; }

  template <class U> bool operator==(const shared_ptr<U> &that) const noexcept {
    return m_ptr == that.m_ptr;
  }

  template <class U> bool operator!=(const shared_ptr<U> &that) const noexcept {
    return m_ptr != that.m_ptr;
  }

  template <class U> bool operator<(const shared_ptr<U> &that) const noexcept {
    return m_ptr < that.m_ptr;
  }

  template <class U> bool operator<=(const shared_ptr<U> &that) const noexcept {
    return m_ptr <= that.m_ptr;
  }

  template <class U> bool operator>(const shared_ptr<U> &that) const noexcept {
    return m_ptr > that.m_ptr;
  }

  template <class U> bool operator>=(const shared_ptr<U> &that) const noexcept {
    return m_ptr >= that.m_ptr;
  }

  template <class U>
  bool owner_before(const shared_ptr<U> &that) const noexcept {
    return m_owner < that.m_owner;
  }

  template <class U>
  bool owner_equal(const shared_ptr<U> &that) const noexcept {
    return m_owner == that.m_owner;
  }

  void swap(shared_ptr &that) noexcept {
    std::swap(m_ptr, that.m_ptr);
    std::swap(m_owner, that.m_owner);
  }

  T *get() const noexcept { return m_ptr; }

  T *operator->() const noexcept { return m_ptr; }

  std::add_lvalue_reference_t<T> operator*() const noexcept { return *m_ptr; }

  explicit operator bool() const noexcept { return m_ptr != nullptr; }
};

template <typename T>
inline shared_ptr<T> make_shared_fused(T *ptr, sp_cnt *owner) noexcept {
  return shared_ptr<T>(ptr, owner);
}

template <typename T> class shared_ptr<T[]> : public shared_ptr<T> {
public:
  using shared_ptr<T>::shared_ptr;

  std::add_lvalue_reference_t<T> operator[](std::size_t i) {
    return this->get()[i];
  }
};

template <typename T> struct enable_shared_from_this {
private:
  sp_cnt *m_owner;

protected:
  enable_shared_from_this() noexcept : m_owner(nullptr) {}

  shared_ptr<T const> shared_from_this() const {
    static_assert(std::is_base_of_v<enable_shared_from_this, T>,
                  "must be derived class");
    if (!m_owner)
      throw std::bad_weak_ptr();
    m_owner->inc_ref();
    return make_shared_fused(static_cast<T const *>(this), m_owner);
  }

  template <typename U>
  inline friend void
  setup_enable_shared_from_this_owner(enable_shared_from_this<U> *, sp_cnt *);
};

template <typename U>
inline void setup_enable_shared_from_this_owner(enable_shared_from_this<U> *ptr,
                                                sp_cnt *owner) {
  ptr->m_owner = owner;
}

template <
    typename T,
    std::enable_if_t<std::is_base_of_v<enable_shared_from_this<T>, T>, int> = 0>
void setup_enable_shared_from_this(T *ptr, sp_cnt *owner) {
  setup_enable_shared_from_this_owner(
      static_cast<enable_shared_from_this<T> *>(ptr), owner);
}

template <typename T, typename... Args,
          std::enable_if_t<!std::is_unbounded_array_v<T>, int> = 0>
shared_ptr<T> make_shared(Args &&...args) {
  auto const deleter = [](T *ptr) noexcept { ptr->~T(); };

  using counter = sp_cnt_impl_fused<T, decltype(deleter)>;
  constexpr size_t offset = std::max(alignof(T), sizeof(counter));
  constexpr size_t align = std::max(alignof(T), alignof(counter));
  constexpr size_t size = offset + sizeof(T);

#if __cpp_aligned_new
  void *mem = ::operator new(size, std::align_val_t(align));
  counter *cnt = reinterpret_cast<counter *>(mem);
#elif
  void *mem = ::operator new(size + align);
  counter *cnt =
      reinterpret_cast<counter *>(reinterpret_cast<size_t>(mem) & align);
#endif

  T *object = reinterpret_cast<T *>(reinterpret_cast<char *>(cnt) + offset);
  try {
    new (object) T(std::forward<Args>(args)...);
  } catch (...) {
#if __cpp_aligned_new
    ::operator delete(mem, std::align_val_t(align));
#elif
    ::operator delete(mem);
#endif

    throw;
  }

  new (cnt) counter(object, mem, deleter);
  setup_enable_shared_from_this(object, cnt);
  return make_shared_fused(object, cnt);
}

template <class T, std::enable_if_t<!std::is_unbounded_array_v<T>, int> = 0>
shared_ptr<T> make_shared_for_overwrite() {
  auto const deleter = [](T *ptr) noexcept { ptr->~T(); };
  using counter = sp_cnt_impl_fused<T, decltype(deleter)>;
  constexpr std::size_t offset = std::max(alignof(T), sizeof(counter));
  constexpr std::size_t align = std::max(alignof(T), alignof(counter));
  constexpr std::size_t size = offset + sizeof(T);
#if __cpp_aligned_new
  void *mem = ::operator new(size, std::align_val_t(align));
  counter *cnt = reinterpret_cast<counter *>(mem);
#else
  void *mem = ::operator new(size + align);
  _SpC *cnt =
      reinterpret_cast<_SpC *>(reinterpret_cast<std::size_t>(mem) & align);
#endif
  T *object = reinterpret_cast<T *>(reinterpret_cast<char *>(cnt) + offset);
  try {
    new (object) T;
  } catch (...) {
#if __cpp_aligned_new
    ::operator delete(mem, std::align_val_t(align));
#else
    ::operator delete(mem);
#endif
    throw;
  }
  new (cnt) counter(object, mem, deleter);
  setup_enable_shared_from_this(object, cnt);
  return make_shared_fused(object, cnt);
}

template <class T, class... Args,
          std::enable_if_t<std::is_unbounded_array_v<T>, int> = 0>
shared_ptr<T> make_shared(std::size_t len) {
  std::remove_extent_t<T> *p = new std::remove_extent_t<T>[len];
  try {
    return shared_ptr<T>(p);
  } catch (...) {
    delete[] p;
    throw;
  }
}

template <class T, std::enable_if_t<std::is_unbounded_array_v<T>, int> = 0>
shared_ptr<T> make_shared_for_overwrite(std::size_t len) {
  std::remove_extent_t<T> *p = new std::remove_extent_t<T>[len];
  try {
    return shared_ptr<T>(p);
  } catch (...) {
    delete[] p;
    throw;
  }
}

template <class T, class U>
shared_ptr<T> static_pointer_cast(shared_ptr<U> const &ptr) {
  return shared_ptr<T>(ptr, static_cast<T *>(ptr.get()));
}

template <class T, class U>
shared_ptr<T> const_pointer_cast(shared_ptr<U> const &ptr) {
  return shared_ptr<T>(ptr, const_cast<T *>(ptr.get()));
}

template <class T, class U>
shared_ptr<T> reinterpret_pointer_cast(shared_ptr<U> const &ptr) {
  return shared_ptr<T>(ptr, reinterpret_cast<T *>(ptr.get()));
}

template <class T, class U>
shared_ptr<T> dynamic_pointer_cast(shared_ptr<U> const &ptr) {
  T *p = dynamic_cast<T *>(ptr.get());
  if (p != nullptr) {
    return shared_ptr<T>(ptr, p);
  } else {
    return nullptr;
  }
}

} // namespace mstl

#endif // !__RAII__
