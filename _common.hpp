#ifndef __COMMON__
#define __COMMON__

#include <algorithm>
#include <type_traits>
#include <utility>
#include <version>

// C++20 concepts
#if __cpp_concepts && __cpp_lib_concepts
#include <concepts>
#endif

// C++20 三路比较
#ifdef __cpp_lib_three_way_comparison
#include <compare>
#endif

// 迭代器类别约束宏 - 根据编译器支持选择不同实现
#if __cpp_concepts && __cpp_lib_concepts
// C++20 concepts版本：简洁的概念约束
#define _LIBPENGCXX_REQUIRES_ITERATOR_CATEGORY(__category, _Type)              \
  __category _Type
#else
// C++17 SFINAE版本：使用enable_if实现类型约束
#define _LIBPENGCXX_REQUIRES_ITERATOR_CATEGORY(__category, _Type)              \
  class _Type, std::enable_if_t<std::is_convertible_v<                         \
                   typename std::iterator_traits<_Type>::iterator_category,    \
                   __category##_tag>>
#endif

// 透明比较器约束宏 - 确保比较器支持异构比较
#define _LIBPENGCXX_REQUIRES_TRANSPARENT_COMPARE(_Compare, _Tv, _Tp)           \
  class _Compare##Tp = _Compare,                                               \
        class = typename _Compare##Tp::is_transparent,                         \
        class =                                                                \
            decltype(std::declval<bool &>() = std::declval<_Compare##Tp>()(    \
                         std::declval<_Tv>(), std::declval<_Tp>()) =           \
                         std::declval<_Compare##Tp>()(std::declval<_Tp>(),     \
                                                      std::declval<_Tv>()))

// 越界异常抛出宏 - 统一的越界错误处理
#define _LIBPENGCXX_THROW_OUT_OF_RANGE(__i, __n)                               \
  throw std::runtime_error("out of range at index " + std::to_string(__i) +    \
                           ", size " + std::to_string(__n))

// 不可达代码宏 - 针对不同编译器优化提示
#if defined(_MSC_VER)
#define _LIBPENGCXX_UNREACHABLE() __assume(0)
#elif defined(__clang__)
#define _LIBPENGCXX_UNREACHABLE() __builtin_unreachable()
#elif defined(__GNUC__)
#define _LIBPENGCXX_UNREACHABLE() __builtin_unreachable()
#else
#define _LIBPENGCXX_UNREACHABLE()                                              \
  do {                                                                         \
  } while (1)
#endif

// 比较操作符定义宏 - 根据C++20支持生成不同的比较操作符
#if __cpp_lib_three_way_comparison
// C++20版本：使用三路比较和自动生成的操作符
#define _LIBPENGCXX_DEFINE_COMPARISON(_Type)                                   \
  bool operator==(_Type const &__that) const noexcept {                        \
    return std::equal(this->begin(), this->end(), __that.begin(),              \
                      __that.end());                                           \
  }                                                                            \
                                                                               \
  auto operator<=>(_Type const &__that) const noexcept {                       \
    return std::lexicographical_compare_three_way(                             \
        this->begin(), this->end(), __that.begin(), __that.end());             \
  }
#else
// C++17版本：手动定义所有比较操作符
#define _LIBPENGCXX_DEFINE_COMPARISON(_Type)                                   \
  bool operator==(_Type const &__that) const noexcept {                        \
    return std::equal(this->begin(), this->end(), __that.begin(),              \
                      __that.end());                                           \
  }                                                                            \
                                                                               \
  bool operator!=(_Type const &__that) const noexcept {                        \
    return !(*this == __that);                                                 \
  }                                                                            \
                                                                               \
  bool operator<(_Type const &__that) const noexcept {                         \
    return std::lexicographical_compare(this->begin(), this->end(),            \
                                        __that.begin(), __that.end());         \
  }                                                                            \
                                                                               \
  bool operator>(_Type const &__that) const noexcept {                         \
    return __that < *this;                                                     \
  }                                                                            \
                                                                               \
  bool operator<=(_Type const &__that) const noexcept {                        \
    return !(__that < *this);                                                  \
  }                                                                            \
                                                                               \
  bool operator>=(_Type const &__that) const noexcept {                        \
    return !(*this < __that);                                                  \
  }
#endif

#endif // !__COMMON__