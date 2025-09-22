#include "raii.hpp"
#include <iostream>
#include <vector>

// unique

struct MyClass {
  int a, b, c;
};

struct Animal {
  virtual void speak() = 0;
  virtual ~Animal() = default;
};

struct Dog : Animal {
  int age;

  Dog(int age_) : age(age_) {}

  virtual void speak() { printf("Bark! I'm %d Year Old!\n", age); }
};

struct Cat : Animal {
  int &age;

  Cat(int &age_) : age(age_) {}

  virtual void speak() { printf("Meow! I'm %d Year Old!\n", age); }
};

// shared
// CRTP
struct Student : mstl::enable_shared_from_this<Student> {
  const char *name;
  int age;

  explicit Student(const char *name_, int age_) : name(name_), age(age_) {
    std::cout << "Student 构造\n";
  }

  Student(Student &&) = delete;

  void func() { std::cout << (void *)shared_from_this().get() << '\n'; }

  ~Student() { std::cout << "Student 析构\n"; }
};

struct StudentDerived : Student {
  explicit StudentDerived(const char *name_, int age_) : Student(name_, age_) {
    std::cout << "StudentDerived 构造\n";
  }

  ~StudentDerived() { std::cout << "StudentDerived 析构\n"; }
};

int main() {
  // unique
  std::vector<mstl::unique_ptr<Animal>> zoo;
  int age = 3;
  zoo.push_back(mstl::make_unique<Cat>(age));
  zoo.push_back(mstl::make_unique<Dog>(age));
  for (auto const &a : zoo) {
    a->speak();
  }
  age++;
  for (auto const &a : zoo) {
    a->speak();
  }

  // shared
  mstl::shared_ptr<Student> p0(new StudentDerived("FurryMonster", 20));
  auto dp = mstl::static_pointer_cast<StudentDerived>(p0);
  mstl::shared_ptr<Student const> bp = p0;
  p0 = mstl::const_pointer_cast<Student>(bp);
  std::cout << dp->name << '\n';

  mstl::shared_ptr<Student> p =
      mstl::make_shared<Student>("FurryMonster", 20); // make_shared 一次性创建
  mstl::shared_ptr<Student> p2(
      new Student("FurryMonster", 20)); // 用 new 创建指针后再构造（不推荐）
  mstl::shared_ptr<Student> p3(new Student("FurryMonster", 20),
                               [](Student *p) { delete p; });
  Student *raw_p = p.get();          // 获取原始指针
  mstl::shared_ptr<Student> p4 = p;  // 浅拷贝
  mstl::shared_ptr<Student> p5 = p3; // 浅拷贝

  p5->func();

  p3 = p5;

  std::cout << p->name << ", " << p->age << '\n';
  std::cout << raw_p->name << ", " << raw_p->age << '\n';
  return 0;
}