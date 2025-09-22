#ifndef __LIST__
#define __LIST__

#include "_common.hpp"
#include <concepts>
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <utility>

namespace mstl {
template <typename T> struct list_base_node {
  list_base_node *m_prev;
  list_base_node *m_next;

  inline T &value();
  inline const T &value() const;
};

template <typename T> struct list_value_node : list_base_node<T> {
  union {
    T m_value;
  };
};

template <typename T> inline T &list_base_node<T>::value() {
  return static_cast<list_value_node<T> &>(*this).m_value;
}

template <typename T> inline const T &list_base_node<T>::value() const {
  return static_cast<const list_value_node<T> &>(*this).m_value;
}

template <typename T, typename Alloc = std::allocator<T>> class list {
public:
  using value_type = T;
  using allocator_type = Alloc;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using pointer = T *;
  using const_pointer = T const *;
  using reference = T &;
  using const_reference = T const &;

  struct iterator;
  struct const_iterator;

private:
  using ListNode = list_base_node<T>;
  using NodeAlloc = typename std::allocator_traits<
      Alloc>::template rebind_alloc<list_base_node<T>>;

  ListNode m_dummy;
  std::size_t m_size;
  [[no_unique_address]] Alloc m_alloc;

  ListNode *allocate() {
    NodeAlloc node_alloc{m_alloc};
    return std::allocator_traits<NodeAlloc>::allocate(node_alloc, 1);
  }

  void deallocate(ListNode *node) noexcept {
    NodeAlloc node_alloc{m_alloc};
    std::allocator_traits<NodeAlloc>::deallocate(
        node_alloc, static_cast<list_value_node<T> *>(node), 1);
  }

  template <typename... Args> void construct_at(T *addr, Args &&...args) {
#if __cpp_lib_constexpr_dynamic_alloc >= 201907L
    std::construct_at(addr, std::forward<Args>(args)...);
#else
    new (addr) T(std::forward<Args>(args)...);
#endif
  }

  void destroy_at(T *addr) noexcept {
#if __cpp_lib_constexpr_dynamic_alloc >= 201907L
    std::destroy_at(addr);
#else
    addr->~T();
#endif
  }

public:
  list() noexcept {
    m_size = 0;
    m_dummy.m_prev = m_dummy.m_next = &m_dummy;
  }

  explicit list(const Alloc &allocator) noexcept : m_alloc(allocator) {
    m_size = 0;
    m_dummy.m_prev = m_dummy.m_next = &m_dummy;
  }

  explicit list(size_t n, const Alloc &allocator = Alloc())
      : m_alloc(allocator) {
    uninit_assign(n);
  }

  explicit list(size_t n, const T &default_val,
                const Alloc &allocator = Alloc())
      : m_alloc(allocator) {
    uninit_assign(n, default_val);
  }

  template <std::input_iterator InputIt>
  list(InputIt first, InputIt last, const Alloc &allocator = Alloc())
      : m_alloc(allocator) {
    uninit_assign(first, last);
  }

  list(std::initializer_list<T> ilist, const Alloc &allocator = Alloc())
      : list(ilist.begin(), ilist.end(), allocator) {}

public:
  list(list &&that) noexcept { uninit_move_assign(std::move(that)); }

  list(list &&that, const Alloc &allocator) noexcept : m_alloc(allocator) {
    uninit_move_assign(std::move(that));
  }

  list &operator=(list &&that) {
    m_alloc = std::move(that.m_alloc);
    clear();
    uninit_move_assign(std::move(that));
  }

  list(const list &that) : m_alloc(that.m_alloc) {
    uninit_assign(that.cbegin(), that.cend());
  }

  list(const list &that, const Alloc &allocator) : m_alloc(allocator) {
    uninit_assign(that.cbegin(), that.cend());
  }

  list &operator=(const list &that) { assign(that.cbegin(), that.cend()); }

  list &operator=(std::initializer_list<T> ilist) { assign(ilist); }

  ~list() noexcept { clear(); }

public:
  bool empty() const noexcept {
    return m_dummy.m_next == &m_dummy && m_dummy.m_prev == &m_dummy;
  }

  size_t size() const noexcept { return m_size; }

  constexpr size_t max_size() const noexcept {
    return std::numeric_limits<size_t>::max();
  }

  Alloc get_allocator() const noexcept { return m_alloc; }

public:
  void clear() noexcept {
    ListNode *cur = m_dummy.m_next;
    while (cur != &m_dummy) {
      destroy_at(&cur->value());
      auto next = cur->m_next;
      deallocate(cur);
      cur = next;
    }
    m_dummy.m_prev = m_dummy.m_next = &m_dummy;
    m_size = 0;
  }

  template <std::input_iterator InputIt>
  void assign(InputIt first, InputIt last) {
    clear();
    uninit_assign(first, last);
  }

  void assign(std::initializer_list<T> ilist) {
    clear();
    uninit_assign(ilist.begin(), ilist.end());
  }

  void assign(size_t n, const T &default_val) {
    clear();
    uninit_assign(n, default_val);
  }

  void push_back(const T &val) { emplace_back(val); }

  void push_back(T &&val) { emplace_back(std::move(val)); }

  void push_front(const T &val) { emplace_front(val); }

  void push_front(T &&val) { emplace_front(std::move(val)); }

  template <typename... Args> T &emplace_back(Args &&...args) {
    ListNode *node = allocate();
    ListNode *prev = m_dummy.prev;
    prev->m_next = node;
    node->m_prev = prev;
    node->m_next = &m_dummy;
    construct_at(&node->value(), std::forward<Args>(args)...);
    m_dummy.m_prev = node;
    ++m_size;
    return node->value();
  }

  template <typename... Args> T &emplace_front(Args &&...args) {
    ListNode *node = allocate();
    ListNode *next = m_dummy.m_next;
    next->m_prev = node;
    node->m_next = next;
    node->m_prev = &m_dummy;
    construct_at(&node->value(), std::forward<Args>(args)...);
    m_dummy.m_next = node;
    ++m_size;
    return node->value();
  }

public:
  T &front() noexcept { return m_dummy.m_next->value(); }
  const T &front() const noexcept { return m_dummy.m_next->value(); }

  T &back() noexcept { return m_dummy.m_prev->value(); }
  const T &back() const noexcept { return m_dummy.m_prev->value(); }

  iterator begin() noexcept { return iterator(m_dummy.m_next); }
  const_iterator begin() const noexcept {
    return const_iterator(m_dummy.m_next);
  }
  const iterator cbegin() const noexcept {
    return const_iterator(m_dummy.m_next);
  }

  iterator end() noexcept { return iterator(&m_dummy); }
  const_iterator end() const noexcept { return const_iterator(&m_dummy); }
  const_iterator cend() const noexcept { return const_iterator(&m_dummy); }

  using reverse_iterator = std::reverse_iterator<iterator>;
  using reverse_const_iterator = std::reverse_iterator<const_iterator>;

  

public:
  struct iterator {
  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = T;
    using difference_type = ptrdiff_t;
    using pointer = T *;
    using reference = T &;

  private:
    ListNode *m_cur;

    friend list;

    explicit iterator(ListNode *cur) noexcept : m_cur(cur) {}

  public:
    iterator() = default;

    iterator &operator++() noexcept { //++iterator
      m_cur = m_cur->m_next;
      return *this;
    }

    iterator &operator++(int) noexcept { // iterator++
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    iterator &operator--() noexcept { // --iterator
      m_cur = m_cur->m_prev;
      return *this;
    }

    iterator &operator--(int) noexcept { // iterator--
      auto tmp = *this;
      --*this;
      return tmp;
    }

    T &operator*() const noexcept { return m_cur->value(); }

    bool operator!=(const iterator &that) const noexcept {
      return m_cur != that.m_cur;
    }

    bool operator==(const iterator &that) const noexcept {
      return m_cur == that.m_cur;
    }
  };

  struct const_iterator {
  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = T;
    using difference_type = ptrdiff_t;
    using pointer = T const *;
    using reference = T const &;

  private:
    ListNode *m_cur;

    friend list;

    explicit const_iterator(ListNode *cur) noexcept : m_cur(cur) {}

  public:
    const_iterator() = default;

    const_iterator(iterator that) noexcept : m_cur(that.m_cur) {}

    explicit operator iterator() noexcept {
      return iterator{const_cast<ListNode *>(m_cur)};
    }

    const_iterator &operator++() noexcept {
      m_cur = m_cur->m_next;
      return *this;
    }

    const_iterator &operator++(int) noexcept {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    const_iterator &operator--() noexcept {
      m_cur = m_cur->m_prev;
      return *this;
    }

    const_iterator &operator--(int) noexcept {
      auto tmp = *this;
      --*this;
      return tmp;
    }

    const T &operator*() const noexcept { return m_cur->value(); }

    bool operator!=(const const_iterator &that) const noexcept {
      return m_cur != that.m_cur;
    }

    bool operator==(const const_iterator &that) const noexcept {
      return !(*this != that);
    }
  };

private:
  void uninit_move_assign(list &&that) {
    auto prev = that.m_dummy.m_prev;
    auto next = that.m_dummy.m_next;
    prev->m_next = &m_dummy;
    next->m_prev = &m_dummy;
    m_dummy = that.m_dummy;
    that.m_dummy.m_prev = that.m_dummy.m_next = &that.m_dummy;
    m_size = that.m_size;
    that.m_size = 0;
  }

  template <std::input_iterator InputIt>
  void uninit_assign(InputIt first, InputIt last) {
    m_size = 0;
    ListNode *prev = &m_dummy;
    while (first != last) {
      ListNode *node = allocate();
      prev->m_next = node;
      node->m_prev = prev;
      construct_at(&node->value(), *first);
      prev = node;
      ++first;
      ++m_size;
    }
    m_dummy.m_prev = prev;
    prev->m_next = &m_dummy;
  }

  template <typename... Args>
    requires std::constructible_from<T, Args...>
  void uninit_assign(size_t n, Args &&...args) {
    ListNode *prev = &m_dummy;
    for (size_t i = 0; i < n; i++) {
      ListNode *node = allocate();
      prev->m_next = node;
      node->m_prev = prev;
      construct_at(&node->value(), std::forward<Args>(args)...);
      prev = node;
    }
    m_dummy.m_prev = prev;
    prev->m_next = &m_dummy;
    m_size = n;
  }

public:
  _LIBPENGCXX_DEFINE_COMPARISON(list);
};

} // namespace mstl

#endif // !__LIST__