#include "variant.hpp"
#include <iostream>

void print(mstl::variant<std::string, int, double> v) {
    v.visit([&](auto v) { std::cout << v << '\n'; });
    // if (v.holds_alternative<std::string>()) {
    //     std::cout << v.get<std::string>() << '\n';
    // } else if (v.holds_alternative<int>()) {
    //     std::cout << v.get<int>() << '\n';
    // } else if (v.holds_alternative<double>()) {
    //     std::cout << v.get<double>() << '\n';
    // }
}

int main() {
    mstl::variant<std::string, int, double> v1(mstl::in_place_idx<0>, "asas");
    print(v1);
    mstl::variant<std::string, int, double> v2(42);
    print(v2);
    mstl::variant<std::string, int, double> v3(3.14);
    print(v3);
}
