# 第 5 章 练习

共 5 题。先自己动手，再到 `solutions/` 对照（文字题答案在 `solutions/answers.md`，代码题答案是可编译运行的源码）。

## 练习 1（判断命中哪个版本）

已知正文 `ex02` 的 `Box` 主模板与 `Box<T*>` 偏特化，判断下面三个实例化各命中哪个版本，并说明理由：

```cpp
Box<double> a;
Box<double*> b;
Box<const char*> c;
```

特别想清楚 `c`：`const char*` 算不算"指针模式"？

## 练习 2（读报错）

编译下面这段代码（节选）得到的实测报错如下：

```cpp
template <typename T, int N>
class FixedArray { T data_[N]; };

int main() { FixedArray<int> fa; return 0; }
```

```
err2.cpp: In function 'int main()':
err2.cpp:3:28: error: wrong number of template arguments (1, should be 2)
err2.cpp:2:7: note: provided for 'template<class T, int N> class FixedArray'
```

按正文的三步套路解读：病根在哪一行、错的是什么、怎么改？

## 练习 3（写代码）

在正文 `ex01` 的基础上，再给 `typeName` 增加一个 `int` 的全特化，返回字符串 `"integer"`。验证：`typeName(42)` 输出 integer，`typeName(4.2)` 仍输出 generic type，`typeName("dds")` 仍输出 C string 特化版。

## 练习 4（写代码）

仿照 `printAll` 的递归骨架，写可变参数模板 `sumAll`，把任意多个 int 相加返回。验证：`sumAll(1, 2, 3, 4)` 得 10，`sumAll(7)` 得 7，`sumAll()` 得 0。

## 练习 5（思考题）

函数模板**不能**偏特化。可是工作中你写了个通用函数模板，又想给"指针类型"一份不同实现。通行的解法是什么？（提示：回想第 4 章的重载决议，以及第 3 章我们怎样给同名函数安排不同签名。）
