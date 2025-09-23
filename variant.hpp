#ifndef __VARIANT__
#define __VARIANT__

#include <algorithm>
#include <cstddef>
#include <exception>
#include <functional>
#include <type_traits>
#include <utility>

namespace mstl {

template <size_t I> struct in_place_index {
    explicit in_place_index() = default;
};
template <size_t I> constexpr in_place_index<I> in_place_idx;

struct bad_variant_access : std::exception {
    bad_variant_access() = default;
    virtual ~bad_variant_access() = default;

    const char *what() const noexcept override { return "Bad Variant Access."; }
};

template <typename, typename> struct variant_index;

template <typename, size_t> struct variant_type;

template <typename... Ts> struct variant {
  private:
    size_t m_index;
    alignas(std::max({alignof(Ts)...})) char m_union[std::max({sizeof(Ts)...})];

    using destructor_function = void (*)(char *) noexcept;

    static destructor_function *destructors_table() noexcept {
        static destructor_function func_ptrs[sizeof...(Ts)] = {
            [](char *union_ptr) noexcept {
                reinterpret_cast<Ts *>(union_ptr)->~Ts();
            }...};
        return func_ptrs;
    }

    using copy_constructor_function = void (*)(char *, const char *) noexcept;

    static copy_constructor_function *copy_constructors_table() noexcept {
        static copy_constructor_function func_ptrs[sizeof...(Ts)] = {
            [](char *union_dst, const char *union_src) noexcept {
                new (union_dst) Ts(*reinterpret_cast<const Ts *>(union_src));
            }...};
        return func_ptrs;
    }

    using copy_assignment_function = void (*)(char *, const char *) noexcept;

    static copy_assignment_function *copy_assignments_table() noexcept {
        static copy_assignment_function func_ptrs[sizeof...(Ts)] = {
            [](char *union_dst, const char *union_src) noexcept {
                *reinterpret_cast<Ts *>(union_dst) =
                    *reinterpret_cast<const Ts *>(union_src);
            }...};
        return func_ptrs;
    }

    using move_constructor_function = void (*)(char *, char *) noexcept;

    static move_constructor_function *move_constructors_table() noexcept {
        static move_constructor_function func_ptrs[] = {[](char *union_dst,
                                                           char *union_src) {
            new (union_dst) Ts(std::move(*reinterpret_cast<Ts *>(union_src)));
        }...};
        return func_ptrs;
    }

    using move_assignment_function = void (*)(char *, char *) noexcept;

    static move_assignment_function *move_assignments_table() noexcept {
        static move_assignment_function func_ptrs[] = {
            [](char *union_dst, char *union_src) {
                *reinterpret_cast<Ts *>(union_dst) =
                    std::move(*reinterpret_cast<Ts *>(union_src));
            }...};
        return func_ptrs;
    }

    template <typename Lambda>
    using const_visitor_function = std::common_type<typename std::invoke_result<
        Lambda, const Ts &>::type...>::type (*)(const char *, Lambda &&);

    template <typename Lambda>
    static const_visitor_function<Lambda> *const_visitors_table() noexcept {
        static const_visitor_function<Lambda> func_ptrs[sizeof...(Ts)] = {
            [](const char *union_ptr, Lambda &&l) ->
            typename std::common_type<typename std::invoke_result<
                Lambda, const Ts &>::type...>::type {
                return std::invoke(std::forward<Lambda>(l),
                                   *reinterpret_cast<const Ts *>(union_ptr));
            }...};
        return func_ptrs;
    }

    template <typename Lambda>
    using visitor_function = std::common_type<typename std::invoke_result<
        Lambda, Ts &>::type...>::type (*)(char *, Lambda &&);

    template <typename Lambda>
    static visitor_function<Lambda> *visitors_table() noexcept {
        static visitor_function<Lambda> func_ptrs[sizeof...(Ts)] = {
            [](char *union_ptr, Lambda &&l) ->
            typename std::common_type<
                typename std::invoke_result<Lambda, Ts &>::type...>::type {
                return std::invoke(std::forward<Lambda>(l),
                                   *reinterpret_cast<Ts *>(union_ptr));
            }...};
        return func_ptrs;
    }

  public:
    template <typename T, typename std::enable_if<
                              std::disjunction<std::is_same<T, Ts>...>::value,
                              int>::type = 0>
    variant(T value) : m_index(variant_index<variant, T>::value) {
        T *p = reinterpret_cast<T *>(m_union);
        new (p) T(value);
    }

    variant(const variant &that) : m_index(that.m_index) {
        copy_constructors_table()[index()](m_union, that.m_union);
    }

    variant &operator=(const variant &that) {
        m_index = that.m_index;
        copy_assignments_table()[index()](m_union, that.m_union);
        return *this;
    }

    variant(variant &&that) : m_index(that.m_index) {
        move_constructors_table()[index()](m_union, that.m_union);
    }

    variant &operator=(variant &&that) {
        m_index = that.m_index;
        move_assignments_table()[index()](m_union, that.m_union);
        return *this;
    }

    template <size_t I, typename... Args>
    explicit variant(in_place_index<I>, Args &&...args) : m_index(I) {
        new (m_union) typename variant_type<variant, I>::type(
            std::forward<Args>(args)...);
    }

    ~variant() noexcept { destructors_table()[index()](m_union); }

    template <typename Lambda>
    std::common_type<typename std::invoke_result<Lambda, Ts &>::type...>::type
    visit(Lambda &&lambda) {
        // TODO: not implemented yet.
        return visitors_table<Lambda>()[index()](m_union,
                                                 std::forward<Lambda>(lambda));
    }

    template <typename Lambda>
    std::common_type<
        typename std::invoke_result<Lambda, const Ts &>::type...>::type
    visit(Lambda &&lambda) const {
        // TODO: not implemented yet.
        return const_visitors_table<Lambda>()[index()](
            m_union, std::forward<Lambda>(lambda));
    }

    constexpr size_t index() const noexcept { return m_index; }

    template <typename T> constexpr bool holds_type() const noexcept {
        return variant_index<variant, T>::value == index();
    }

    template <size_t I> typename variant_type<variant, I>::type &get() {
        static_assert(I < sizeof...(Ts), "i is out of range.");
        if (m_index != I)
            throw bad_variant_access();
        return *reinterpret_cast<typename variant_type<variant, I>::type *>(
            m_union);
    }

    template <typename T> T &get() {
        return get<variant_index<variant, T>::value>();
    }

    template <size_t I>
    const typename variant_type<variant, I>::type &get() const {
        static_assert(I < sizeof...(Ts), "i is out of range.");
        if (m_index != I)
            throw bad_variant_access();
        return *reinterpret_cast<
            const typename variant_type<variant, I>::type *>(m_union);
    }

    template <typename T> const T &get() const {
        return get<variant_index<variant, T>::value>();
    }

    template <size_t I> typename variant_type<variant, I>::type *get_if() {
        static_assert(I < sizeof...(Ts), "i is out of range.");
        if (m_index != I)
            return nullptr;
        return reinterpret_cast<typename variant_type<variant, I>::type *>(
            m_union);
    }

    template <typename T> T *get_if() {
        return get_if<variant_index<variant, T>::value>();
    }

    template <size_t I>
    const typename variant_type<variant, I>::type *get_if() const {
        static_assert(I < sizeof...(Ts), "i is out of range.");
        if (m_index != I)
            return nullptr;
        return reinterpret_cast<
            const typename variant_type<variant, I>::type *>(m_union);
    }

    template <typename T> const T *get_if() const {
        return get_if<variant_index<variant, T>::value>();
    }
};

template <typename T, typename... Ts>
struct variant_type<variant<T, Ts...>, 0> {
    using type = T;
};

template <typename T, typename... Ts, size_t I>
struct variant_type<variant<T, Ts...>, I> {
    using type = typename variant_type<variant<Ts...>, I - 1>::type;
};

template <typename T, typename... Ts>
struct variant_index<variant<T, Ts...>, T> {
    static constexpr size_t value = 0;
};

template <typename T0, typename T, typename... Ts>
struct variant_index<variant<T0, Ts...>, T> {
    static constexpr size_t value = variant_index<variant<Ts...>, T>::value + 1;
};

} // namespace mstl

#endif
