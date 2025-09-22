#ifndef __LIST__
#define __LIST__

#include <cstddef>
#include <memory>
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

private:
  using ListNode = list_base_node<T>;
  using AllocNode = typename std::allocator_traits<
      Alloc>::template rebind_alloc<list_base_node<T>>;

  ListNode m_dummy;
  std::size_t m_size;
  [[no_unique_address]] Alloc m_alloc;

  ListNode *new_node() {
    AllocNode allocNode{m_alloc};
    return std::allocator_traits<AllocNode>::allocate(allocNode, 1);
  }

  void delete_node(ListNode *node) noexcept {
    AllocNode allocNode{m_alloc};
    std::allocator_traits<AllocNode>::deallocate(
        allocNode, static_cast<list_value_node<T> *>(node), 1);
  }

public:
};

} // namespace mstl

#endif // !__LIST__