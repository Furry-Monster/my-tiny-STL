#ifndef __SET__
#define __SET__

#include "_common.hpp"
#include "_rbtree.hpp"
#include <cstddef>
#include <functional>
#include <memory>
#include <utility>

namespace mstl {

template <class _Tp, class _Compare = std::less<_Tp>,
          class _Alloc = std::allocator<_Tp>>
struct set : _RbTreeImpl<_Tp const, _Compare, _Alloc> {
    using typename _RbTreeImpl<_Tp const, _Compare, _Alloc>::const_iterator;
    using typename _RbTreeImpl<_Tp const, _Compare, _Alloc>::node_type;
    using iterator = const_iterator;
    using value_type = _Tp;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    set() = default;

    explicit set(_Compare __comp)
        : _RbTreeImpl<_Tp const, _Compare, _Alloc>(__comp) {}

    set(set &&) = default;
    set &operator=(set &&) = default;

    set(set const &__that) : _RbTreeImpl<_Tp const, _Compare, _Alloc>() {
        this->_M_single_insert(__that.begin(), __that.end());
    }

    set &operator=(set const &__that) {
        if (&__that != this) {
            this->assign(__that.begin(), __that.end());
        }
        return *this;
    }

    set &operator=(std::initializer_list<_Tp> __ilist) {
        this->assign(__ilist);
        return *this;
    }

    void assign(std::initializer_list<_Tp> __ilist) {
        this->clear();
        this->_M_single_insert(__ilist.begin(), __ilist.end());
    }

    _Compare value_comp() const noexcept { return this->_M_comp; }

    template <class _Tv,
              _LIBPENGCXX_REQUIRES_TRANSPARENT_COMPARE(_Compare, _Tv, _Tp)>
    const_iterator find(_Tp &&__value) const noexcept {
        return this->_M_find(__value);
    }

    const_iterator find(_Tp const &__value) const noexcept {
        return this->_M_find(__value);
    }

    std::pair<iterator, bool> insert(_Tp &&__value) {
        return this->_M_single_emplace(std::move(__value));
    }

    std::pair<iterator, bool> insert(_Tp const &__value) {
        return this->_M_single_emplace(__value);
    }

    template <class... _Ts>
    std::pair<iterator, bool> emplace(_Ts &&...__value) {
        return this->_M_single_emplace(std::forward<_Ts>(__value)...);
    }

    template <_LIBPENGCXX_REQUIRES_ITERATOR_CATEGORY(std::input_iterator,
                                                     _InputIt)>
    void insert(_InputIt __first, _InputIt __last) {
        return this->_M_single_insert(__first, __last);
    }

    using _RbTreeImpl<_Tp const, _Compare, _Alloc>::assign;

    template <_LIBPENGCXX_REQUIRES_ITERATOR_CATEGORY(std::input_iterator,
                                                     _InputIt)>
    void assign(_InputIt __first, _InputIt __last) {
        this->clear();
        return this->_M_single_insert(__first, __last);
    }

    using _RbTreeImpl<_Tp const, _Compare, _Alloc>::erase;

    template <class _Tv,
              _LIBPENGCXX_REQUIRES_TRANSPARENT_COMPARE(_Compare, _Tv, _Tp)>
    std::size_t erase(_Tv &&__value) {
        return this->_M_single_erase(__value);
    }

    std::size_t erase(_Tp const &__value) {
        return this->_M_single_erase(__value);
    }

    template <class _Tv,
              _LIBPENGCXX_REQUIRES_TRANSPARENT_COMPARE(_Compare, _Tv, _Tp)>
    std::size_t count(_Tv &&__value) const noexcept {
        return this->_M_contains(__value) ? 1 : 0;
    }

    std::size_t count(_Tp const &__value) const noexcept {
        return this->_M_contains(__value) ? 1 : 0;
    }

    template <class _Tv,
              _LIBPENGCXX_REQUIRES_TRANSPARENT_COMPARE(_Compare, _Tv, _Tp)>
    bool contains(_Tv &&__value) const noexcept {
        return this->_M_contains(__value);
    }

    bool contains(_Tp const &__value) const noexcept {
        return this->_M_contains(__value);
    }

    template <class _Tv,
              _LIBPENGCXX_REQUIRES_TRANSPARENT_COMPARE(_Compare, _Tv, _Tp)>
    node_type extract(_Tv &&__value) {
        iterator __it = this->_M_find(__value);
        return __it != this->end() ? this->extract(__it) : node_type();
    }

    node_type extract(_Tp const &__value) {
        iterator __it = this->_M_find(__value);
        return __it != this->end() ? this->extract(__it) : node_type();
    }
};

template <class _Tp, class _Compare = std::less<_Tp>,
          class _Alloc = std::allocator<_Tp>>
struct multi_set : _RbTreeImpl<_Tp const, _Compare, _Alloc> {
    using typename _RbTreeImpl<_Tp const, _Compare, _Alloc>::const_iterator;
    using typename _RbTreeImpl<_Tp const, _Compare, _Alloc>::node_type;
    using iterator = const_iterator;
    using value_type = _Tp;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    multi_set() = default;

    explicit multi_set(_Compare __comp)
        : _RbTreeImpl<_Tp const, _Compare, _Alloc>(__comp) {}

    multi_set(multi_set &&) = default;
    multi_set &operator=(multi_set &&) = default;

    multi_set(multi_set const &__that)
        : _RbTreeImpl<_Tp const, _Compare, _Alloc>() {
        this->_M_multi_insert(__that.begin(), __that.end());
    }

    multi_set &operator=(multi_set const &__that) {
        if (&__that != this) {
            this->assign(__that.begin(), __that.end());
        }
        return *this;
    }

    multi_set &operator=(std::initializer_list<_Tp> __ilist) {
        this->assign(__ilist);
        return *this;
    }

    void assign(std::initializer_list<_Tp> __ilist) {
        this->clear();
        this->_M_multi_insert(__ilist.begin(), __ilist.end());
    }

    _Compare value_comp() const noexcept { return this->_M_comp; }

    template <class _Tv,
              _LIBPENGCXX_REQUIRES_TRANSPARENT_COMPARE(_Compare, _Tv, _Tp)>
    const_iterator find(_Tp &&__value) const noexcept {
        return this->_M_find(__value);
    }

    const_iterator find(_Tp const &__value) const noexcept {
        return this->_M_find(__value);
    }

    iterator insert(_Tp &&__value) {
        return this->_M_multi_emplace(std::move(__value));
    }

    iterator insert(_Tp const &__value) {
        return this->_M_multi_emplace(__value);
    }

    template <class... _Ts> iterator emplace(_Ts &&...__value) {
        return this->_M_multi_emplace(std::forward<_Ts>(__value)...);
    }

    template <_LIBPENGCXX_REQUIRES_ITERATOR_CATEGORY(std::input_iterator,
                                                     _InputIt)>
    void insert(_InputIt __first, _InputIt __last) {
        return this->_M_multi_insert(__first, __last);
    }

    using _RbTreeImpl<_Tp const, _Compare, _Alloc>::assign;

    template <_LIBPENGCXX_REQUIRES_ITERATOR_CATEGORY(std::input_iterator,
                                                     _InputIt)>
    void assign(_InputIt __first, _InputIt __last) {
        this->clear();
        return this->_M_multi_insert(__first, __last);
    }

    using _RbTreeImpl<_Tp const, _Compare, _Alloc>::erase;

    template <class _Tv,
              _LIBPENGCXX_REQUIRES_TRANSPARENT_COMPARE(_Compare, _Tv, _Tp)>
    std::size_t erase(_Tv &&__value) {
        return this->_M_multi_erase(__value);
    }

    std::size_t erase(_Tp const &__value) {
        return this->_M_multi_erase(__value);
    }

    template <class _Tv,
              _LIBPENGCXX_REQUIRES_TRANSPARENT_COMPARE(_Compare, _Tv, _Tp)>
    std::size_t count(_Tv &&__value) const noexcept {
        return this->_M_multi_count(__value);
    }

    std::size_t count(_Tp const &__value) const noexcept {
        return this->_M_multi_count(__value);
    }

    template <class _Tv,
              _LIBPENGCXX_REQUIRES_TRANSPARENT_COMPARE(_Compare, _Tv, _Tp)>
    bool contains(_Tv &&__value) const noexcept {
        return this->_M_contains(__value);
    }

    bool contains(_Tp const &__value) const noexcept {
        return this->_M_contains(__value);
    }

    template <class _Tv,
              _LIBPENGCXX_REQUIRES_TRANSPARENT_COMPARE(_Compare, _Tv, _Tp)>
    node_type extract(_Tv &&__value) {
        iterator __it = this->_M_find(__value);
        return __it != this->end() ? this->extract(__it) : node_type();
    }

    node_type extract(_Tp const &__value) {
        iterator __it = this->_M_find(__value);
        return __it != this->end() ? this->extract(__it) : node_type();
    }
};

} // namespace mstl

#endif // !__SET__
