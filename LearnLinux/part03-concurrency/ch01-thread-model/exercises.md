# 第 1 章 练习（第三部分）

共 5 题。先自己动手，再到 `solutions/` 对照（文字题答案在 `solutions/answers.md`，代码题答案是可编译运行的源码）。

## 练习 1（生命周期判断）

下面三段代码，哪些能平安结束、哪些会出事（何种事）？逐段给出结论与依据。

```cpp
// A
{ std::thread t(work); }                       // work 约需 100ms

// B
std::thread t(work); t.join(); t.join();

// C
std::thread t(work); t.detach();
// 此后对 t 不再做任何操作，t 在作用域结束时析构
```

## 练习 2（读代码：detach 后的悬垂引用）

```cpp
void startWorker() {
    int progress = 0;
    std::thread t([&progress] {
        for (int i = 0; i < 100; ++i) { progress = i; /* 耗时工作 */ }
    });
    t.detach();
}   // startWorker 返回
```

这段代码埋了什么雷？它和正文里"detach 必须保证所用之物活得比线程久"是什么关系？给出两种修法（一种改数据归属，一种改回收方式）。

## 练习 3（写代码：分段并行求和）

写程序：4 个 std::thread 分段求 1..1000000 的和（分槽汇总，不许共享累加），主线程合计后打印，并与单线程直接求和的结果对照，两者必须一致（500000500000），不一致时以非 0 退出码报警。

## 练习 4（写代码：pthread 版分段求和）

把练习 3 改用 pthread 实现：每段的部分和由工作函数计算、经返回值（void*）交回主线程汇总。注意参数结构体与结果内存的归属管理。

## 练习 5（思考题）

(a) detach 的正当用途是什么？举一个车载/服务场景，并说明用它时必须额外操心哪两件事。
(b) 标准为什么让 joinable 的 std::thread 析构时 terminate，而不是自动帮你 join 或 detach？各设想一种隐式行为会掩盖的事故。
