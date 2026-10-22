# 第 6 章 练习

共 5 题。先自己动手，再到 `solutions/` 对照（文字题答案在 `solutions/answers.md`，代码题答案是可编译运行的源码）。

## 练习 1（读代码：找出所有权事故）

```cpp
int* makeValue() {
    return new int(42);
}

int main() {
    int* p = makeValue();
    int* q = p;          // 两个裸指针指向同一块内存
    delete p;
    delete q;            // 事故发生
    return 0;
}
```

问题有两层：这次运行会出什么事（回想第 2 章的工具怎么"看"它）；把所有权模型换掉的话，用哪个智能指针、代码要怎么改，才能从结构上杜绝它？

## 练习 2（预测引用计数）

```cpp
std::shared_ptr<int> a(new int(1));
// 步骤 1
std::shared_ptr<int> b = a;
// 步骤 2
{
    std::shared_ptr<int> c = b;
    // 步骤 3
    a.reset();
    // 步骤 4
}
// 步骤 5
b.reset();
// 步骤 6
```

写出步骤 1～6 每一步之后，底层 int 对象的引用计数分别是多少，并指出对象在哪一步被真正销毁。（`reset()` 使该 shared_ptr 放手，计数减一。）

## 练习 3（写代码）

写一个工厂函数 `createBlock(int v)`，返回 `std::unique_ptr<DataBlock>`（`DataBlock` 自行定义，含一个 int 字段并打印创建/销毁日志）。在 main 中接住它，再用 `std::move` 把所有权转移给另一个 `unique_ptr`，断言源已为空，程序结束时确认销毁日志出现且只出现一次。

## 练习 4（写代码）

下面这个类只有拷贝构造（逻辑与正文 `Buffer` 同型，已简化）。给它补上**移动构造**，并写一个按值传参的 `consume` 函数，用计数器验证：传左值时走拷贝、传 `std::move(...)` 时走移动、且被移动后源的 `size()` 为 0。

```cpp
class SimpleBuffer {
public:
    explicit SimpleBuffer(std::size_t n);
    SimpleBuffer(const SimpleBuffer& other);   // 深拷贝，计数 ++copies
    ~SimpleBuffer();
    std::size_t size() const;
    static int copies;
    static int moves;                          // 现在恒为 0，等你补完构造它才会动
};
```

## 练习 5（思考题）

(a) `weak_ptr` 为什么不让使用者直接解引用，而必须先 `lock()`？如果允许直接解引用，会有什么竞态/安全问题（单线程下也算）？
(b) 一句话向同事解释：为什么你给移动构造加的 `noexcept` 能让 `vector<你的类>` 扩容时改走移动而不是拷贝？
