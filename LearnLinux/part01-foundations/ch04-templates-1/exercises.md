# 第 4 章 练习

共 5 题。先自己动手，再到 `solutions/` 对照（文字题答案在 `solutions/answers.md`，代码题答案是可编译运行的源码）。

已知正文的模板：

```cpp
template <typename T>
T maxValue(T a, T b) { return a > b ? a : b; }
```

## 练习 1（判断：哪些调用编译不过，为什么）

```cpp
maxValue(3, 9);
maxValue(3, 9.5);
maxValue('a', 'z');
maxValue(3.0, 9);
maxValue<double>(3, 9);
```

逐个判断：能编译的，T 是什么、结果是什么；不能编译的，卡在哪一步。

## 练习 2（推导练习）

```cpp
template <typename T> void f(T x);
template <typename T> void g(T& x);

double d = 0;
const double cd = 0;
char name[8] = "can";
```

写出下面每个调用里 T 的推导结果：

```cpp
f(d);   f(cd);   f(name);
g(d);   g(cd);   g(name);
```

## 练习 3（写代码）

把第 3 章练习里的 `clampValue` 升级成函数模板 `clampT`：一份代码同时服务 int 和 double。验证：`clampT(150, 0, 100)` 得 100，`clampT(2.7, 0.0, 2.5)` 得 2.5，`clampT(42, 0, 100)` 得 42。

## 练习 4（写代码）

写类模板 `MinMax<T>`：构造时存入两个 T 类型的值，提供 `getMin()` 与 `getMax()`。用 `MinMax<int>(17, 42)` 和 `MinMax<double>(3.5, 2.25)` 验证输出。

## 练习 5（思考题）

同事把一个函数模板的定义写在 `util.cpp` 里、只在 `util.h` 放声明，另一个文件包含头文件调用它，编译能过、链接却报 `undefined reference`。用本章学到的"实例化"解释：问题出在哪一步？通常的改法是什么？
