#ifndef __ARRAY__
#define __ARRAY__

#include "_common.hpp"
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>

namespace mstl {

template <typename T, std::size_t N> struct array {
    using value_type = T;
    using size_type = std::size_t;
    using diff_type = std::ptrdiff_t;
    using pointer = T *;
    using const_pointer = const T *;
    using reference = T &;
    using const_reference = const T &;
    using iterator = T *;
    using const_iterator = const T *;
    using rev_iterator = std::reverse_iterator<T *>;
    using const_rev_iterator = std::reverse_iterator<const T *>;

    T m_arr[N];

    reference operator[](size_t i) noexcept { return m_arr[i]; }
    const_reference operator[](size_t i) const noexcept { return m_arr[i]; }

    reference at(size_t i) {
        if (i >= N) [[unlikely]]
            _LIBPENGCXX_THROW_OUT_OF_RANGE(i, N);
        return m_arr[i];
    }
    const_reference at(size_t i) const {
        if (i >= N) [[unlikely]]
            _LIBPENGCXX_THROW_OUT_OF_RANGE(i, N);
        return m_arr[i];
    }

    void fill(T const &val) noexcept(std::is_nothrow_copy_assignable_v<T>) {
        for (size_t i = 0; i < N; i++)
            m_arr[i] = val;
    }

    void swap(array &that) noexcept(std::is_nothrow_swappable_v<T>) {
        for (size_t i = 0; i < N; i++) {
            std::swap(m_arr[i], that.m_arr[i]);
        }
    }

    reference front() noexcept { return m_arr[0]; }
    const_reference front() const noexcept { return m_arr[0]; }

    reference back() noexcept { return m_arr[N - 1]; }
    const_reference back() const noexcept { return m_arr[N - 1]; }

    static constexpr bool empty() noexcept { return false; }
    static constexpr size_t size() noexcept { return N; }
    static constexpr size_t max_size() noexcept { return N; }

    const_pointer cdata() const noexcept { return m_arr; }
    pointer data() noexcept { return m_arr; }
    const_pointer data() const noexcept { return m_arr; }

    const_iterator cbegin() const noexcept { return m_arr; }
    const_iterator cend() const noexcept { return m_arr + N; } // range in [0,N)
    iterator begin() noexcept { return m_arr; }
    const_iterator begin() const noexcept { return m_arr; }
    iterator end() noexcept { return m_arr + N; }
    const_iterator end() const noexcept { return m_arr + N; }

    const_rev_iterator crbegin() const noexcept {
        return const_rev_iterator(end());
    }
    const_rev_iterator crend() const noexcept {
        return const_rev_iterator(begin());
    }
    const_rev_iterator rbegin() const noexcept {
        return const_rev_iterator(end());
    }
    const_rev_iterator rend() const noexcept {
        return const_rev_iterator(begin());
    }
    rev_iterator rbegin() noexcept { return rev_iterator(end()); }
    rev_iterator rend() noexcept { return rev_iterator(begin()); }

    _LIBPENGCXX_DEFINE_COMPARISON(array);
};

template <typename T> struct array<T, 0> {
    using value_type = T;
    using size_type = std::size_t;
    using diff_type = std::ptrdiff_t;
    using pointer = T *;
    using const_pointer = const T *;
    using reference = T &;
    using const_reference = const T &;
    using iterator = T *;
    using const_iterator = const T *;
    using rev_iterator = std::reverse_iterator<T *>;
    using const_rev_iterator = std::reverse_iterator<const T *>;

    T m_arr[0];

    T &operator[](size_t __i) noexcept { _LIBPENGCXX_UNREACHABLE(); }

    T const &operator[](size_t __i) const noexcept {
        _LIBPENGCXX_UNREACHABLE();
    }

    T &at(size_t __i) { throw std::out_of_range("array::at"); }

    T const &at(size_t __i) const { throw std::out_of_range("array::at"); }

    void fill(T const &) noexcept {}

    void swap(array &) noexcept {}

    T &front() noexcept { _LIBPENGCXX_UNREACHABLE(); }

    T const &front() const noexcept { _LIBPENGCXX_UNREACHABLE(); }

    T &back() noexcept { _LIBPENGCXX_UNREACHABLE(); }

    T const &back() const noexcept { _LIBPENGCXX_UNREACHABLE(); }

    static constexpr bool empty() noexcept { return true; }

    static constexpr size_t size() noexcept { return 0; }

    static constexpr size_t max_size() noexcept { return 0; }

    T const *cdata() const noexcept { return nullptr; }

    T const *data() const noexcept { return nullptr; }

    T *data() noexcept { return nullptr; }

    T const *cbegin() const noexcept { return nullptr; }

    T const *cend() const noexcept { return nullptr; }

    T const *begin() const noexcept { return nullptr; }

    T const *end() const noexcept { return nullptr; }

    T *begin() noexcept { return nullptr; }

    T *end() noexcept { return nullptr; }

    T const *crbegin() const noexcept { return nullptr; }

    T const *crend() const noexcept { return nullptr; }

    T const *rbegin() const noexcept { return nullptr; }

    T const *rend() const noexcept { return nullptr; }

    T *rbegin() noexcept { return nullptr; }

    T *rend() noexcept { return nullptr; }

    _LIBPENGCXX_DEFINE_COMPARISON(array);
};

// deduce guide!
template <typename Tfirst, typename... Tothers,
          typename = std::enable_if_t<(std::is_same_v<Tfirst, Tothers> && ...)>>
array(Tfirst, Tothers...) -> array<Tfirst, 1 + sizeof...(Tothers)>;

} // namespace mstl

#endif // !__ARRAY__
