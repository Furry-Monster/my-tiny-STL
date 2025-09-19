#include "function.hpp"
#include <cstdio>
#include <cstdlib>

void func_hello(int i) { printf("#%d Hello\n", i); }

struct func_printnum_t {
  void operator()(int i) const { printf("#%d Numbers are: %d, %d\n", i, x, y); }
  int x;
  int y;
};

void repeat_twice(function<void(int)> const &func) {
  func(1);
  func(2);
}

int main(int argc, const char **argv) {
  int x = 4;
  int y = 2;
  repeat_twice([=](int i) { printf("#%d Numbers are: %d, %d\n", i, x, y); });
  func_printnum_t func_printnum{x, y};
  repeat_twice(func_printnum);
  repeat_twice(func_hello);

  function<void(int)> &&f{[](int i) { printf("i=%d\n", i); }};

  f(2);
  function<void(int)> ff{f};
  f = nullptr;
  ff(3);

  return 0;
}