#ifndef __MAP__
#define __MAP__

#include "_common.hpp"
#include "_rbtree.hpp"
#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>

namespace mstl {

template <class _Compare, class _Value, class = void>
struct _RbTreeValueCompare {
  protected:
    [[no_unique_address]] _Compare _M_comp;

  public:
    _RbTreeValueCompare(_Compare __comp = _Compare()) noexcept
        : _M_comp(__comp) {}

    bool operator()(typename _Value::first_type const &__lhs,
                    _Value const &__rhs) const noexcept {
        return this->_M_comp(__lhs, __rhs.first);
    }

    bool operator()(_Value const &__lhs,
                    typename _Value::first_type const &__rhs) const noexcept {
        return this->_M_comp(__lhs.first, __rhs);
    }

    bool operator()(_Value const &__lhs, _Value const &__rhs) const noexcept {
        return this->_M_comp(__lhs.first, __rhs.first);
    }

    struct _RbTreeIsMap;
};

template <class _Compare, class _Value>
struct _RbTreeValueCompare<
    _Compare, _Value,
    decltype((void)static_cast<typename _Compare::is_transparent *>(nullptr))> {
    [[no_unique_address]] _Compare _M_comp;

    _RbTreeValueCompare(_Compare __comp = _Compare()) noexcept
        : _M_comp(__comp) {}

    template <class _Lhs>
    bool operator()(_Lhs &&__lhs, _Value const &__rhs) const noexcept {
        return this->_M_comp(__lhs, __rhs.first);
    }

    template <class _Rhs>
    bool operator()(_Value const &__lhs, _Rhs &&__rhs) const noexcept {
        return this->_M_comp(__lhs.first, __rhs);
    }

    bool operator()(_Value const &__lhs, _Value const &__rhs) const noexcept {
        return this->_M_comp(__lhs.first, __rhs.first);
    }

    using is_transparent = typename _Compare::is_transparent;
};

template <class _Key, class _Mapped, class _Compare = std::less<_Key>,
          class _Alloc = std::allocator<std::pair<_Key const, _Mapped>>>
struct map
    : _RbTreeImpl<std::pair<_Key const, _Mapped>,
                  _RbTreeValueCompare<_Compare, std::pair<_Key const, _Mapped>>,
                  _Alloc> {
    using key_type = _Key;
    using mapped_type = _Mapped;
    using value_type = std::pair<_Key const, _Mapped>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

  private:
    using _ValueComp = _RbTreeValueCompare<_Compare, value_type>;

  public:
    using typename _RbTreeImpl<value_type, _ValueComp, _Alloc>::iterator;
    using typename _RbTreeImpl<value_type, _ValueComp, _Alloc>::const_iterator;
    using typename _RbTreeImpl<value_type, _ValueComp, _Alloc>::node_type;

    map() = default;

    explicit map(_Compare __comp)
        : _RbTreeImpl<value_type, _ValueComp, _Alloc>(__comp) {}

    map(std::initializer_list<value_type> __ilist) {
        _M_single_insert(__ilist.begin(), __ilist.end());
    }

    explicit map(std::initializer_list<value_type> __ilist, _Compare __comp)
        : _RbTreeImpl<value_type, _ValueComp, _Alloc>(__comp) {
        _M_single_insert(__ilist.begin(), __ilist.end());
    }

    template <_LIBPENGCXX_REQUIRES_ITERATOR_CATEGORY(std::input_iterator,
                                                     _InputIt)>
    explicit map(_InputIt __first, _InputIt __last) {
        _M_single_insert(__first, __last);
    }

    template <_LIBPENGCXX_REQUIRES_ITERATOR_CATEGORY(std::input_iterator,
                                                     _InputIt)>
    explicit map(_InputIt __first, _InputIt __last, _Compare __comp)
        : _RbTreeImpl<value_type, _ValueComp, _Alloc>(__comp) {
        _M_single_insert(__first, __last);
    }

    map(map &&) = default;
    map &operator=(map &&) = default;

    map(map const &__that) : _RbTreeImpl<value_type, _ValueComp, _Alloc>() {
        this->_M_single_insert(__that.begin(), __that.end());
    }

    map &operator=(map const &__that) {
        if (&__that != this) {
            this->assign(__that.begin(), __that.end());
        }
        return *this;
    }

    map &operator=(std::initializer_list<value_type> __ilist) {
        this->assign(__ilist);
        return *this;
    }

    void assign(std::initializer_list<value_type> __ilist) {
        this->clear();
        this->_M_single_insert(__ilist.begin(), __ilist.end());
    }

    _Compare key_comp() const noexcept { return this->_M_comp->_M_comp; }

    _ValueComp value_comp() const noexcept { return this->_M_comp; }

    template <class _Kv, _LIBPENGCXX_REQUIRES_TRANSPARENT_COMPARE(
                             _ValueComp, _Kv, value_type)>
    iterator find(_Kv &&__key) noexcept {
        return this->_M_find(__key);
    }

    template <class _Kv, _LIBPENGCXX_REQUIRES_TRANSPARENT_COMPARE(
                             _ValueComp, _Kv, value_type)>
    const_iterator find(_Kv &&__key) const noexcept {
        return this->_M_find(__key);
    }

    iterator find(_Key const &__key) noexcept { return this->_M_find(__key); }

    const_iterator find(_Key const &__key) const noexcept {
        return this->_M_find(__key);
    }

    std::pair<iterator, bool> insert(value_type &&__value) {
        return this->_M_single_emplace(std::move(__value));
    }

    std::pair<iterator, bool> insert(value_type const &__value) {
        return this->_M_single_emplace(__value);
    }

    template <class _Kv, _LIBPENGCXX_REQUIRES_TRANSPARENT_COMPARE(
                             _ValueComp, _Kv, value_type)>
    _Mapped const &at(_Kv const &__key) const {
        const_iterator __it = this->_M_find(__key);
        if (__it == this->end()) [[unlikely]] {
            throw std::out_of_range("map::at");
        }
        return __it->second;
    }

    template <class _Kv, _LIBPENGCXX_REQUIRES_TRANSPARENT_COMPARE(
                             _ValueComp, _Kv, value_type)>
    _Mapped &at(_Kv const &__key) {
        iterator __it = this->_M_find(__key);
        if (__it == this->end()) [[unlikely]] {
            throw std::out_of_range("map::at");
        }
        return __it->second;
    }

    _Mapped const &at(_Key const &__key) const {
        const_iterator __it = this->_M_find(__key);
        if (__it == this->end()) [[unlikely]] {
            throw std::out_of_range("map::at");
        }
        return __it->second;
    }

    _Mapped &at(_Key const &__key) {
        iterator __it = this->_M_find(__key);
        if (__it == this->end()) [[unlikely]] {
            throw std::out_of_range("map::at");
        }
        return __it->second;
    }

    template <class _Kv, _LIBPENGCXX_REQUIRES_TRANSPARENT_COMPARE(
                             _ValueComp, _Kv, value_type)>
    _Mapped &operator[](_Kv const &__key) {
        iterator __it = this->_M_find(__key);
        if (__it == this->end()) {
            __it = this->_M_single_emplace(std::piecewise_construct,
                                           std::forward_as_tuple(__key),
                                           std::forward_as_tuple())
                       .first;
        }
        return __it->second;
    }

    _Mapped &operator[](_Key const &__key) {
        iterator __it = this->_M_find(__key);
        if (__it == this->end()) {
            __it = this->_M_single_emplace(std::piecewise_construct,
                                           std::forward_as_tuple(__key),
                                           std::forward_as_tuple())
                       .first;
        }
        return __it->second;
    }

    template <class _Mp,
              class = std::enable_if_t<std::is_convertible_v<_Mp, _Mapped>>>
    std::pair<iterator, bool> insert_or_assign(_Key const &__key,
                                               _Mp &&__mapped) {
        std::pair<iterator, bool> __result = this->_M_single_emplace(
            std::piecewise_construct, std::forward_as_tuple(__key),
            std::forward_as_tuple(std::forward<_Mp>(__mapped)));
        if (!__result.second) {
            __result.first->second = std::forward<_Mp>(__mapped);
        }
        return __result;
    }

    template <class _Mp,
              class = std::enable_if_t<std::is_convertible_v<_Mp, _Mapped>>>
    std::pair<iterator, bool> insert_or_assign(_Key &&__key, _Mp &&__mapped) {
        std::pair<iterator, bool> __result = this->_M_single_emplace(
            std::piecewise_construct, std::forward_as_tuple(std::move(__key)),
            std::forward_as_tuple(std::forward<_Mp>(__mapped)));
        if (!__result.second) {
            __result.first->second = std::forward<_Mp>(__mapped);
        }
        return __result;
    }

    template <class... Vs> std::pair<iterator, bool> emplace(Vs &&...__value) {
        return this->_M_single_emplace(std::forward<Vs>(__value)...);
    }

    template <class... _Ms>
    std::pair<iterator, bool> try_emplace(_Key &&__key, _Ms &&...__mapped) {
        return this->_M_single_emplace(
            std::piecewise_construct, std::forward_as_tuple(std::move(__key)),
            std::forward_as_tuple(std::forward<_Ms>(__mapped)...));
    }

    template <class... _Ms>
    std::pair<iterator, bool> try_emplace(_Key const &__key,
                                          _Ms &&...__mapped) {
        return this->_M_single_emplace(
            std::piecewise_construct, std::forward_as_tuple(__key),
            std::forward_as_tuple(std::forward<_Ms>(__mapped)...));
    }

    template <_LIBPENGCXX_REQUIRES_ITERATOR_CATEGORY(std::input_iterator,
                                                     _InputIt)>
    iterator insert(_InputIt __first, _InputIt __last) {
        return this->_M_single_insert(__first, __last);
    }

    using _RbTreeImpl<value_type, _ValueComp, _Alloc>::assign;

    template <_LIBPENGCXX_REQUIRES_ITERATOR_CATEGORY(std::input_iterator,
                                                     _InputIt)>
    iterator assign(_InputIt __first, _InputIt __last) {
        this->clear();
        return this->_M_single_insert(__first, __last);
    }

    using _RbTreeImpl<value_type, _ValueComp, _Alloc>::erase;

    template <class _Kv, _LIBPENGCXX_REQUIRES_TRANSPARENT_COMPARE(
                             _ValueComp, _Kv, value_type)>
    size_t erase(_Kv &&__key) {
        return this->_M_single_erase(__key);
    }

    size_t erase(_Key const &__key) { return this->_M_single_erase(__key); }

    template <class _Kv, _LIBPENGCXX_REQUIRES_TRANSPARENT_COMPARE(
                             _ValueComp, _Kv, value_type)>
    size_t count(_Kv &&__value) const noexcept {
        return this->_M_contains(__value) ? 1 : 0;
    }

    size_t count(_Key const &__value) const noexcept {
        return this->_M_contains(__value) ? 1 : 0;
    }

    template <class _Kv, _LIBPENGCXX_REQUIRES_TRANSPARENT_COMPARE(
                             _ValueComp, _Kv, value_type)>
    bool contains(_Kv &&__value) const noexcept {
        return this->_M_contains(__value);
    }

    bool contains(_Key const &__value) const noexcept {
        return this->_M_contains(__value);
    }

    template <class _Kv, _LIBPENGCXX_REQUIRES_TRANSPARENT_COMPARE(
                             _ValueComp, _Kv, value_type)>
    node_type extract(_Kv &&__key) {
        iterator __it = this->_M_find(__key);
        return __it != this->end() ? this->extract(__it) : node_type();
    }

    node_type extract(_Key const &__key) {
        iterator __it = this->_M_find(__key);
        return __it != this->end() ? this->extract(__it) : node_type();
    }
};

template <class _Key, class _Mapped, class _Compare = std::less<_Key>,
          class _Alloc = std::allocator<std::pair<_Key const, _Mapped>>>
struct multi_map
    : _RbTreeImpl<std::pair<_Key const, _Mapped>,
                  _RbTreeValueCompare<_Compare, std::pair<_Key const, _Mapped>>,
                  _Alloc> {
    using key_type = _Key;
    using mapped_type = _Mapped;
    using value_type = std::pair<_Key const, _Mapped>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

  private:
    using _ValueComp = _RbTreeValueCompare<_Compare, value_type>;

  public:
    using typename _RbTreeImpl<value_type, _ValueComp, _Alloc>::iterator;
    using typename _RbTreeImpl<value_type, _ValueComp, _Alloc>::const_iterator;
    using typename _RbTreeImpl<value_type, _ValueComp, _Alloc>::node_type;

    multi_map() = default;

    explicit multi_map(_Compare __comp)
        : _RbTreeImpl<value_type, _ValueComp, _Alloc>(__comp) {}

    multi_map(std::initializer_list<value_type> __ilist) {
        _M_multi_insert(__ilist.begin(), __ilist.end());
    }

    explicit multi_map(std::initializer_list<value_type> __ilist,
                       _Compare __comp)
        : _RbTreeImpl<value_type, _ValueComp, _Alloc>(__comp) {
        _M_multi_insert(__ilist.begin(), __ilist.end());
    }

    template <_LIBPENGCXX_REQUIRES_ITERATOR_CATEGORY(std::input_iterator,
                                                     _InputIt)>
    explicit multi_map(_InputIt __first, _InputIt __last) {
        _M_multi_insert(__first, __last);
    }

    template <_LIBPENGCXX_REQUIRES_ITERATOR_CATEGORY(std::input_iterator,
                                                     _InputIt)>
    explicit multi_map(_InputIt __first, _InputIt __last, _Compare __comp)
        : _RbTreeImpl<value_type, _ValueComp, _Alloc>(__comp) {
        _M_multi_insert(__first, __last);
    }

    multi_map(multi_map &&) = default;
    multi_map &operator=(multi_map &&) = default;

    multi_map(multi_map const &__that)
        : _RbTreeImpl<value_type, _ValueComp, _Alloc>() {
        this->_M_multi_insert(__that.begin(), __that.end());
    }

    multi_map &operator=(multi_map const &__that) {
        if (&__that != this) {
            this->assign(__that.begin(), __that.end());
        }
        return *this;
    }

    multi_map &operator=(std::initializer_list<value_type> __ilist) {
        this->assign(__ilist);
        return *this;
    }

    void assign(std::initializer_list<value_type> __ilist) {
        this->clear();
        this->_M_multi_insert(__ilist.begin(), __ilist.end());
    }

    _Compare key_comp() const noexcept { return this->_M_comp->_M_comp; }

    _ValueComp value_comp() const noexcept { return this->_M_comp; }

    template <class _Kv, _LIBPENGCXX_REQUIRES_TRANSPARENT_COMPARE(
                             _ValueComp, _Kv, value_type)>
    iterator find(_Kv &&__key) noexcept {
        return this->_M_find(__key);
    }

    template <class _Kv, _LIBPENGCXX_REQUIRES_TRANSPARENT_COMPARE(
                             _ValueComp, _Kv, value_type)>
    const_iterator find(_Kv &&__key) const noexcept {
        return this->_M_find(__key);
    }

    iterator find(_Key const &__key) noexcept { return this->_M_find(__key); }

    const_iterator find(_Key const &__key) const noexcept {
        return this->_M_find(__key);
    }

    std::pair<iterator, bool> insert(value_type &&__value) {
        return this->_M_single_emplace(std::move(__value));
    }

    std::pair<iterator, bool> insert(value_type const &__value) {
        return this->_M_single_emplace(__value);
    }

    template <class... _Ts>
    std::pair<iterator, bool> emplace(_Ts &&...__value) {
        return this->_M_single_emplace(std::forward<_Ts>(__value)...);
    }

    template <class... _Ts>
    std::pair<iterator, bool> try_emplace(_Key &&__key, _Ts &&...__value) {
        return this->_M_single_emplace(
            std::piecewise_construct, std::forward_as_tuple(std::move(__key)),
            std::forward_as_tuple(std::forward<_Ts>(__value)...));
    }

    template <class... _Ts>
    std::pair<iterator, bool> try_emplace(_Key const &__key, _Ts &&...__value) {
        return this->_M_single_emplace(
            std::piecewise_construct, std::forward_as_tuple(__key),
            std::forward_as_tuple(std::forward<_Ts>(__value)...));
    }

    template <_LIBPENGCXX_REQUIRES_ITERATOR_CATEGORY(std::input_iterator,
                                                     _InputIt)>
    iterator insert(_InputIt __first, _InputIt __last) {
        return this->_M_single_insert(__first, __last);
    }

    using _RbTreeImpl<value_type, _ValueComp, _Alloc>::assign;

    template <_LIBPENGCXX_REQUIRES_ITERATOR_CATEGORY(std::input_iterator,
                                                     _InputIt)>
    iterator assign(_InputIt __first, _InputIt __last) {
        this->clear();
        return this->_M_single_insert(__first, __last);
    }

    using _RbTreeImpl<value_type, _ValueComp, _Alloc>::erase;

    template <class _Kv, _LIBPENGCXX_REQUIRES_TRANSPARENT_COMPARE(
                             _ValueComp, _Kv, value_type)>
    size_t erase(_Kv &&__key) {
        return this->_M_single_erase(__key);
    }

    size_t erase(_Key const &__key) { return this->_M_single_erase(__key); }

    template <class _Kv, _LIBPENGCXX_REQUIRES_TRANSPARENT_COMPARE(
                             _ValueComp, _Kv, value_type)>
    size_t count(_Kv &&__value) const noexcept {
        return this->_M_multi_count(__value);
    }

    size_t count(_Key const &__value) const noexcept {
        return this->_M_multi_count(__value);
    }

    template <class _Kv, _LIBPENGCXX_REQUIRES_TRANSPARENT_COMPARE(
                             _ValueComp, _Kv, value_type)>
    bool contains(_Kv &&__value) const noexcept {
        return this->_M_contains(__value);
    }

    bool contains(_Key const &__value) const noexcept {
        return this->_M_contains(__value);
    }

    template <class _Kv, _LIBPENGCXX_REQUIRES_TRANSPARENT_COMPARE(
                             _ValueComp, _Kv, value_type)>
    node_type extract(_Kv &&__key) {
        iterator __it = this->_M_find(__key);
        return __it != this->end() ? this->extract(__it) : node_type();
    }

    node_type extract(_Key const &__key) {
        iterator __it = this->_M_find(__key);
        return __it != this->end() ? this->extract(__it) : node_type();
    }
};
} // namespace mstl

#endif // !__MAP__
