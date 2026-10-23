# 阶段项目一：动态数组 DynArray（第一部分收官）

## 为什么做这个

前六章的知识点一直是分开练的：指针算术、所有权、深拷贝、模板、移动语义。这个项目把它们拧成一股绳——**亲手实现一个简化版 `std::vector`**。实现过一次容器，之后你再用任何标准容器，脑子里都有它内部运转的样子；而车载开发里看第三方库、中间件的模板容器代码，也不再是天书。

## 任务

在 `starter/dynarray.h` 的骨架上完成类模板 `DynArray<T>`，让 `starter/test_dynarray.cpp` 的验收程序全部通过。你只需要改 `dynarray.h` 这一个文件。

### 接口规格（验收程序实际考到的）

| 接口 | 要求 |
|---|---|
| 默认构造 | 空数组：size=0，capacity=0 |
| 析构 | 释放存储，无泄漏（ASan 会查） |
| 拷贝构造 / 拷贝赋值 | **深拷贝**：两个对象各有独立存储，改一个不影响另一个；拷贝赋值要对自赋值（`a = a`）安全 |
| 移动构造 / 移动赋值 | 接管存储，源被搬空（size 与 capacity 都归 0）；目标之后仍可正常 push_back；带 `noexcept` |
| `push_back(const T&)` | 尾部追加；容量不足时**翻倍**增长：0→1→2→4→8 |
| `operator[]` | const 与非 const 两个版本，不做边界检查（与 `std::vector` 一致） |
| `size()` / `capacity()` / `empty()` | 如实反映状态 |
| `begin()` / `end()` | 返回指针（const 与非 const 版），能支撑范围 for |

### 约束

- C++11 基线；`-Wall -Wextra` 零警告。
- 元素存储必须自己用 `new T[]` / `delete[]` 管理，**不许内部套 `std::vector` 或其他容器代劳**。
- 模板实现全部写在头文件里（第 4 章讲过为什么）。
- 不修改验收程序 `test_dynarray.cpp`。

## 验收方法

在你的开发机上进入 `starter/`：

```bash
cmake -S . -B build
cmake --build build
./build/test_dynarray        # 期望：打印 ALL TESTS PASSED，退出码 0
```

再过一遍内存检查（本项目的硬性要求）：

```bash
g++ -std=c++11 -Wall -Wextra -g -fsanitize=address,undefined test_dynarray.cpp -o /tmp/test_dyn
/tmp/test_dyn                # 期望：同样 ALL TESTS PASSED，且没有任何 sanitizer 报告
```

**完成标准**：两个构建都打印 `ALL TESTS PASSED`；ASan/UBSan 零报告；警告为零。

## 推进建议

骨架里的 TODO 顺序就是推荐顺序：先析构与 push_back 的增长逻辑，让第 1 组断言转绿；再补深拷贝（构造、赋值各一份，自赋值别翻车）；之后移动构造/赋值；最后通读一遍，把每个函数对照接口表核对。哪一步卡住，先回对应章节看示例：增长与指针 → 第 1 章；深拷贝与所有权 → 第 2、6 章；移动与 noexcept → 第 6 章；类模板写法 → 第 4 章。

验收程序里的断言失败信息会指出第一个翻车点——从那里开始查，别通读代码猜谜。跑出 sanitizer 报告时，回忆第 2 章的读法：先看第一条 SUMMARY，再对行号。

参考实现在 `solutions/dynarray.h`（同目录的验收程序与 starter 完全一致）。建议先独立完成、两项验收都过了，再去对照参考实现看写法差异——对照着学，比抄着学记得牢。

## 选做（不计验收）

- 给 DynArray 加 `reserve(n)`：预留至少 n 的容量，不改变 size；再加 `clear()`：元素清零但保留容量。想清楚它们各自只动哪几个成员。
- 思考题：为什么骨架把拷贝/移动的**赋值**与**构造**分成四个函数，而不是"赋值调用构造"了事？（提示：赋值面对的是一个已经持有存储的自己。）

## 验收入口补注（2026-10-23）

本项目当初即为验收测试先行，无需补齐；本次将三重验证固化为一键脚本 `tests/run_tests.sh`。补齐时实测：参考实现通过验收、ASan/UBSan 构建无报告、骨架以退出码 134 挂在第一条断言上——与 2026-10-22 的原始验证结果一致。
