#ifndef __FUNCTION__
#define __FUNCTION__

#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>
#include <typeinfo>
#include <utility>

namespace mstl {
template <typename Func> class function {
  static_assert(false, "not a valid function.");
};

template <typename Ret, typename... Args> class function<Ret(Args...)> {
private:
  class function_base {
  public:
    virtual Ret call(Args... args) = 0;
    virtual std::unique_ptr<function_base> clone() const = 0;
    virtual const std::type_info &type() const = 0;
    virtual ~function_base() = default;
  };

  template <typename Fn> class function_impl : public function_base {
  public:
    Fn func;

    template <typename... CArgs>
    explicit function_impl(std::in_place_t, CArgs &&...carried_args)
        : func(std::forward<CArgs>(carried_args)...){};

    Ret call(Args... args) override {
      std::invoke(func, std::forward<Args>(args)...);
    }

    std::unique_ptr<function_base> clone() const override {
      return std::make_unique<function_impl>(std::in_place, func);
    }

    const std::type_info &type() const override { return typeid(Fn); }
  };

  std::unique_ptr<function_base> invocable;

public:
  function() = default;
  function(std::nullptr_t) noexcept : function(){};

  template <typename Fn,
            typename = std::enable_if_t<
                std::is_invocable_r_v<Ret, Fn, Args...> &&
                !std::is_same_v<std::decay_t<Fn>, function<Ret(Args...)>>>>
  function(Fn &&f)
      : invocable(std::make_unique<function_impl<Fn>>(std::in_place,
                                                      std::forward<Fn>(f))) {}

  function(function &&that) = default;
  function(const function &that)
      : invocable(that.invocable ? that.invocable->clone() : nullptr) {}

  function &operator=(function &&that) = default;
  function &operator=(const function &that) {
    invocable = that.invocable ? that.invocable->clone() : nullptr;
  }

  explicit operator bool() const noexcept { return invocable != nullptr; }

  bool operator==(std::nullptr_t) const noexcept {
    return invocable == nullptr;
  }

  bool operator!=(std::nullptr_t) const noexcept {
    return invocable != nullptr;
  }

  Ret operator()(Args... args) const {
    if (!invocable) [[__unlikely__]]
      throw std::bad_function_call();
    return invocable->call(std::forward<Args>(args)...);
  }

  const std::type_info &func_type() const noexcept {
    return invocable ? invocable->type() : typeid(void);
  }

  template <typename Fn> Fn *get() const noexcept {
    return (invocable && typeid(Fn) == invocable->type())
               ? std::addressof(
                     static_cast<function_impl<Fn> *>(invocable.get())->func)
               : nullptr;
  }
};
} // namespace mstl

#endif // !__FUNCTION__