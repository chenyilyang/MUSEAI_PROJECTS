# 第 3 章 练习（第三部分）

共 5 题。先自己动手，再到 `solutions/` 对照（文字题答案在 `solutions/answers.md`，代码题答案是可编译运行的源码）。

## 练习 1（判断：三种 wait 写法）

下面三种等待写法各有什么风险？逐一给出结论。

```cpp
// A：裸 wait，状态在锁外先查
if (!g_ready) { std::unique_lock<std::mutex> lk(g_m); g_cv.wait(lk); }

// B：谓词 + if（只查一次）
std::unique_lock<std::mutex> lk(g_m);
if (!g_ready) g_cv.wait(lk);
// 醒来直接往下走

// C：谓词式 wait
std::unique_lock<std::mutex> lk(g_m);
g_cv.wait(lk, [] { return g_ready; });
```

## 练习 2（读代码：检查与等待之间的缝）

```cpp
// 消费者线程
g_m.lock();
bool empty = g_queue.empty();
g_m.unlock();
if (empty) {
    std::unique_lock<std::mutex> lk(g_m);
    g_cv.wait(lk);            // 等生产者 notify
    // 醒来后取元素……
}
```

生产者是标准写法（锁内 push、解锁后 notify_one）。指出消费者这段代码的时序窗口：在什么交错下通知会丢失、程序会睡死？为什么正文的标准写法（在同一把锁内查谓词 + wait）没有这条缝？

## 练习 3（写代码：BlockingQueue<T>）

把 ex02 的思想抽成无界阻塞队列类模板 `BlockingQueue<T>`：`push(const T&)` 永不阻塞；`bool pop(T&)` 在队列空时睡等，已关闭且取空时返回 false；`close()` 置关闭状态并唤醒所有等待者。主程序用它跑通两组验证：int 队列（1..100，总和 5050）与 std::string 队列（拼出 "hello world"）。

> 这个接口就是本部分阶段项目（任务队列 + 日志库）的地基，方法名与语义请保持一致。

## 练习 4（写代码：StartGate 发令枪）

实现一次性发令枪类 `StartGate`：`wait()` 在门开前阻塞；`open()` 放行所有等待者；门开之后再来 `wait()` 的线程应立即通过。主程序自检：4 个线程在门前等待，主线程 100ms 后检查通过数为 0，再开门，join 后通过数应为 4。

## 练习 5（思考题）

(a) notify_one 与 notify_all 如何选型？各举一个"用错"的具体后果（一个是漏人挂死，一个是无谓惊群）。
(b) 信号量为什么没有丢失唤醒问题？用它对照条件变量，说明"自带状态"换来了什么、又付出了什么。
