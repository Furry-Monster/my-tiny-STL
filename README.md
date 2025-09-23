# Monster STL

个人学习用 STL，大部分代码逻辑和https://github.com/parallel101/stl1weekend相同，个人进行了少数优化和改写。

一个轻量级的 C++20 STL 实现，包含常用数据结构和智能指针的自定义实现。

## 特性

- **C++20 标准兼容**：使用现代 C++ 特性实现
- **头文件库**：仅包含头文件，易于集成
- **完整测试**：每个组件都有对应的测试文件
- **高性能**：优化的数据结构实现

## 包含的组件

### 容器类

- **`vector.hpp`** - 动态数组容器
- **`list.hpp`** - 双向链表容器
- **`array.hpp`** - 固定大小数组容器
- **`map.hpp`** - 基于红黑树的关联容器（键值对）
- **`set.hpp`** - 基于红黑树的集合容器

### 智能指针 (RAII)

- **`raii.hpp`** - 智能指针实现
  - `unique_ptr` - 独占所有权智能指针
  - `shared_ptr` - 共享所有权智能指针（支持引用计数）
  - 自定义删除器支持

### 函数对象

- **`function.hpp`** - 可调用对象包装器

### 内部实现

- **`_rbtree.hpp`** - 红黑树实现（map 和 set 的底层数据结构）
- **`_common.hpp`** - 公共工具和定义

## 构建和测试

### 前置要求

- C++20 兼容的编译器（如 GCC 10+, Clang 10+）
- Make 构建工具

### 构建所有测试

```bash
make all
```

### 运行所有测试

```bash
make test
```

### 构建单个组件测试

```bash
make vector_test    # 构建 vector 测试
make list_test      # 构建 list 测试
make map_test       # 构建 map 测试
make set_test       # 构建 set 测试
make raii_test      # 构建 RAII 测试
make function_test  # 构建 function 测试
make array_test     # 构建 array 测试
```

### 调试构建

```bash
make debug
```

### 清理构建文件

```bash
make clean
```

### 查看帮助

```bash
make help
```

## 使用示例

### Vector 使用

```cpp
#include "vector.hpp"

mstl::vector<int> vec;
vec.push_back(1);
vec.push_back(2);
vec.push_back(3);

for (auto& elem : vec) {
    std::cout << elem << " ";
}
```

### Map 使用

```cpp
#include "map.hpp"

mstl::map<std::string, int> m;
m["hello"] = 1;
m["world"] = 2;

for (auto& [key, value] : m) {
    std::cout << key << ": " << value << std::endl;
}
```

### 智能指针使用

```cpp
#include "raii.hpp"

// unique_ptr
auto ptr = mstl::make_unique<int>(42);

// shared_ptr
auto shared = mstl::make_shared<std::string>("hello");
auto shared2 = shared; // 引用计数增加
```

## 命名空间

所有组件都在 `mstl` 命名空间下。

## 编译器支持

- GCC 10+
- Clang 10+
- MSVC 2019+
