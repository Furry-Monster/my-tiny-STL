#ifndef __VECTOR__
#define __VECTOR__

#include "_common.hpp"
#include <cstddef>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

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

    template <typename... Args> void construct_at(T *ptr, Args &&...args) {
#if __cpp_lib_constexpr_dynamic_alloc >= 201907L
        std::construct_at(ptr, std::forward<Args>(args)...);
#else
        new (ptr) T(std::forward<Args>(args)...);
#endif
    }

    void destroy_at(T *ptr) noexcept {
#if __cpp_lib_constexpr_dynamic_alloc >= 201907L
        std::destroy_at(ptr);
#else
        ptr->~T();
#endif
    }

    // 构造函数
  public:
    vector() noexcept {
        m_data = nullptr;
        m_size = 0;
        m_cap = 0;
    }

    vector(std::initializer_list<T> ilist, const Alloc &allocator = Alloc())
        : vector(ilist.begin(), ilist.end(), allocator) {}

    explicit vector(std::size_t n, const Alloc &allocator = Alloc())
        : m_alloc(allocator) {
        m_data = m_alloc.allocate(n);
        m_cap = m_size = n;
        for (size_t i = 0; i < n; i++) {
            construct_at(&m_data[i]);
        }
    }

    vector(std::size_t n, const T &init_val, const Alloc &allocator = Alloc())
        : m_alloc(allocator) {
        m_data = m_alloc.allocate(n);
        m_cap = m_size = n;
        for (size_t i = 0; i < n; i++) {
            construct_at(&m_data[i], init_val);
        }
    }

    template <
        _LIBPENGCXX_REQUIRES_ITERATOR_CATEGORY(std::random_access_iterator, It)>
    vector(It first, It last, const Alloc &allocator = Alloc())
        : m_alloc(allocator) {
        size_t n = last - first;
        m_data = m_alloc.allocate(n);
        m_cap = m_size = n;
        for (size_t i = 0; i < n; i++) {
            construct_at(&m_data[i], *first);
            ++first;
        }
    }

    ~vector() noexcept {
        for (size_t i = 0; i < m_size; i++) {
            destroy_at(&m_data[i]);
        }
        if (m_cap != 0) {
            m_alloc.deallocate(m_data, m_cap);
        }
    }

    // 深浅拷贝
  public:
    vector(vector &&that) noexcept : m_alloc(std::move(that.m_alloc)) {
        m_data = that.m_data;
        m_size = that.m_size;
        m_cap = that.m_cap;
        that.m_data = nullptr;
        that.m_size = 0;
        that.m_cap = 0;
    }

    vector(vector &&that, const Alloc &allocate) noexcept : m_alloc(allocate) {
        m_data = that.m_data;
        m_size = that.m_size;
        m_cap = that.m_cap;
        that.m_data = nullptr;
        that.m_size = 0;
        that.m_cap = 0;
    }

    vector &operator=(vector &&that) noexcept {
        if (&that == this) [[unlikely]]
            return *this;

        for (size_t i = 0; i < m_size; i++) {
            destroy_at(&m_data[i]);
        }

        if (m_cap != 0) {
            m_alloc.deallocate(m_data, m_cap);
        }

        m_data = that.m_data;
        m_size = that.m_size;
        m_cap = that.m_cap;
        that.m_data = nullptr;
        that.m_size = 0;
        that.m_cap = 0;

        return *this;
    }

    void swap(vector &that) noexcept {
        std::swap(m_data, that.m_data);
        std::swap(m_size, that.m_size);
        std::swap(m_cap, that.m_cap);
        std::swap(m_alloc, that.m_alloc);
    }

    vector(const vector &that) : m_alloc(that.m_alloc) {
        m_cap = m_size = that.m_size;
        if (m_size != 0) {
            m_data = m_alloc.allocate(m_cap);
            for (size_t i = 0; i < m_size; i++) {
                construct_at(&m_data[i], std::as_const(that.m_data[i]));
            }
        } else {
            m_data = nullptr;
        }
    }

    vector(const vector &that, const Alloc &allocate) : m_alloc(allocate) {
        m_cap = m_size = that.m_size;
        if (m_size != 0) {
            m_data = m_alloc.allocate(m_cap);
            for (size_t i = 0; i < m_size; i++) {
                construct_at(&m_data[i], std::as_const(that.m_data[i]));
            }
        } else {
            m_data = nullptr;
        }
    }

    vector &operator=(const vector &that) {
        if (&that == this) [[unlikely]]
            return *this;

        reserve(that.m_size);
        m_size = that.m_size;
        for (size_t i = 0; i < m_size; i++) {
            construct_at(&m_data[i], std::as_const(that.m_data[i]));
        }
        return *this;
    }

    // 内存管理
  public:
    void clear() noexcept {
        for (size_t i = 0; i < m_size; i++) {
            destroy_at(&m_data[i]);
        }
        m_size = 0;
    }

    void resize(size_t n) {
        if (n < m_size) {
            for (size_t i = n; i < m_size; i++) {
                destroy_at(&m_data[i]);
            }
        } else if (n > m_size) {
            reserve(n);
            for (size_t i = m_size; i < n; i++) {
                construct_at(&m_data[i]);
            }
        }
        m_size = n;
    }

    void resize(size_t n, const T &default_val) {
        if (n < m_size) {
            for (size_t i = n; i < m_size; i++) {
                destroy_at(&m_data[i]);
            }
        } else if (n > m_size) {
            reserve(n);
            for (size_t i = m_size; i < n; i++) {
                construct_at(&m_data[i], default_val);
            }
        }
        m_size = n;
    }

    void shrink_to_fit() noexcept {
        auto old_data = m_data;
        auto old_cap = m_cap;
        m_cap = m_size;
        if (m_size == 0) {
            m_data = nullptr;
        } else {
            m_data = m_alloc.allocate(m_size);
        }
        if (old_cap != 0) [[likely]] {
            for (std::size_t i = 0; i != m_size; i++) {
                construct_at(&m_data[i], std::move_if_noexcept(old_data[i]));
                destroy_at(&old_data[i]);
            }
            m_alloc.deallocate(old_data, old_cap);
        }
    }

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
                construct_at(&m_data[i], std::move_if_noexcept(old_data[i]));
            }
            for (size_t i = 0; i < m_size; i++) {
                destroy_at(&old_data[i]);
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
    Alloc get_allocator() const noexcept { return m_alloc; }

    // 访问
  public:
    T &operator[](size_t i) noexcept { return m_data[i]; }
    const T &operator[](size_t i) const noexcept { return m_data[i]; }

    T &at(size_t i) {
        if (i >= m_size) [[unlikely]]
            _LIBPENGCXX_THROW_OUT_OF_RANGE(i, m_size);
        return m_data[i];
    }
    const T &at(size_t i) const {
        if (i >= m_size) [[unlikely]]
            _LIBPENGCXX_THROW_OUT_OF_RANGE(i, m_size);
        return m_data[i];
    }

    T &front() noexcept { return *m_data; }
    const T &front() const noexcept { return *m_data; }

    T &back() noexcept { return m_data[m_size - 1]; }
    const T &back() const noexcept { return m_data[m_size - 1]; }

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

    // 数据操作
  public:
    void push_back(const T &lval) {
        if (m_size + 1 > m_cap) [[unlikely]]
            reserve(m_size + 1);
        construct_at(&m_data[m_size], lval);
        ++m_size;
    }

    void push_back(T &&rval) {
        if (m_size + 1 > m_cap) [[unlikely]]
            reserve(m_size + 1);
        construct_at(&m_data[m_size], std::move(rval));
        ++m_size;
    }

    template <typename... Args> T &emplace_back(Args &&...args) {
        if (m_size + 1 >= m_cap) [[unlikely]]
            reserve(m_size + 1);
        T *addr = &m_data[m_size];
        construct_at(addr, std::forward<Args>(args)...);
        ++m_size;
        return *addr;
    }

    template <typename... Args> T *emplace(const T *it, Args &&...args) {
        size_t j = it - m_data;
        reserve(m_size + 1);
        for (size_t i = m_size; i > j; i--) {
            construct_at(&m_data[i], std::move(m_data[i - 1]));
            destroy_at(&m_data[i - 1]);
        }
        ++m_size;
        construct_at(&m_data[j], std::forward<Args>(args)...);
        return m_data + j;
    }

    T *insert(const T *it, T &&val) {
        size_t j = it - m_data;
        reserve(m_size + 1);
        for (size_t i = m_size; i > j; i--) {
            construct_at(&m_data[i], std::move(m_data[i - 1]));
            destroy_at(&m_data[i - 1]);
        }
        ++m_size;
        construct_at(&m_data[j], std::move(val));
        return m_data + j;
    }

    T *insert(const T *it, size_t n, const T &val) {
        size_t j = it - m_data;
        if (n == 0) [[unlikely]]
            return const_cast<T *>(it);
        reserve(m_size + n);
        for (size_t i = m_size; i > j; i--) {
            construct_at(&m_data[i + n - 1], std::move(m_data[i - 1]));
            destroy_at(&m_data[i - 1]);
        }
        m_size += n;
        for (size_t i = j; i < j + n; i++) {
            construct_at(&m_data[i], val);
        }
        return m_data + j;
    }

    template <
        _LIBPENGCXX_REQUIRES_ITERATOR_CATEGORY(std::random_access_iterator, It)>
    T *insert(const T *it, It first, It last) {
        size_t j = it - m_data;
        size_t n = last - first;
        if (n == 0) [[unlikely]]
            return const_cast<T *>(it);
        reserve(m_size + n);
        for (size_t i = m_size; i > j; i--) {
            construct_at(&m_data[i + n - 1], std::move(m_data[i - 1]));
            destroy_at(&m_data[i - 1]);
        }
        m_size += n;
        for (size_t i = j; i < j + n; i++) {
            construct_at(&m_data[i], *first);
            ++first;
        }
        return m_data + j;
    }

    T *insert(const T *it, std::initializer_list<T> ilist) {
        return insert(it, ilist.begin(), ilist.end());
    }

    void pop_back() noexcept {
        --m_size;
        destroy_at(&m_data[m_size]);
    }

    T *erase(const T *it) noexcept(std::is_nothrow_move_assignable_v<T>) {
        size_t i = it - m_data;
        for (size_t j = i + 1; j < m_size; j++) {
            m_data[j - 1] = std::move(m_data[j]);
        }
        --m_size;
        destroy_at(&m_data[m_size]);
        return const_cast<T *>(it);
    }

    T *erase(T *first, T *last) noexcept(std::is_nothrow_move_assignable_v<T>) {
        size_t diff = last - first;
        for (size_t j = last - m_data; j < m_size; j++) {
            m_data[j - diff] = std::move(m_data[j]);
        }
        m_size -= diff;
        for (size_t j = m_size; j < m_size + diff; j++) {
            destroy_at(&m_data[j]);
        }
        return const_cast<T *>(first);
    }

    // 内存分配
  public:
    void assign(size_t n, const T &default_val) {
        clear();
        reserve(n);
        m_size = n;
        for (size_t i = 0; i < n; i++) {
            construct_at(&m_data[i], default_val);
        }
    }

    template <
        _LIBPENGCXX_REQUIRES_ITERATOR_CATEGORY(std::random_access_iterator, It)>
    void assign(It first, It last) {
        clear();
        size_t n = last - first;
        reserve(n);
        m_size = n;
        for (size_t i = 0; i < n; i++) {
            construct_at(&m_data[i], *first);
            ++first;
        }
    }

    void assign(std::initializer_list<T> ilist) {
        assign(ilist.begin(), ilist.end());
    }

    vector &operator=(std::initializer_list<T> ilist) {
        assign(ilist.begin(), ilist.end());
        return *this;
    }

    // 比较函数
  public:
    _LIBPENGCXX_DEFINE_COMPARISON(vector);
};

} // namespace mstl

#endif // !__VECTOR__
