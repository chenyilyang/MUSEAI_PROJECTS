# 第 2 章 练习

共 5 题。先自己动手，再到 `solutions/` 对照（文字题答案在 `solutions/answers.md`，代码题答案是可编译运行的源码）。

## 练习 1（读代码，写输出）

下面这段程序的输出是什么？逐步推演每一行之后 `a`、`b`、`p` 各是什么状态。

```cpp
#include <cstdio>
int main() {
    int a = 3;
    int* p = &a;
    int** pp = &p;
    **pp += 4;
    int b = 10;
    *pp = &b;
    **pp = 0;
    std::printf("a=%d, b=%d\n", a, b);
    return 0;
}
```

提示：`*pp` 操作的是 `p` 本身（指向），`**pp` 操作的才是 `p` 所指的变量（值）。

## 练习 2（找错误，两处）

下面这段代码有两处内存错误，分别指出并改对：

```cpp
void process() {
    int* arr = new int[10];
    // ... 使用 arr ...
    delete arr;

    int* q = new int(5);
    // ... 使用 *q ...
}   // 函数结束
```

## 练习 3（写代码）

仿照正文 `buildScores` 的模式，写函数 `bool allocArray(int n, int** out)`：在堆上分配 n 个 int、依次填入 1..n，通过 `out` 交回给调用者，成功返回 `true`；`n <= 0` 或 `out` 为 `nullptr` 时返回 `false` 且不分配。在 `main` 里调用它打印数组，最后正确释放。

## 练习 4（写代码）

写函数 `int* cloneArray(const int* src, int n)`：在堆上克隆一份数组并返回。在 `main` 里验证：修改副本的第一个元素，原数组不受影响；最后正确释放副本。

要求：`src` 的指针参数要加 `const`，想想为什么。

## 练习 5（思考题）

正文说"`delete` 之后立刻把指针置 `nullptr`"能防事故。它具体防得住什么、防不住什么？（提示：想想如果有另一个指针也指着同一块内存，会怎么样。）
