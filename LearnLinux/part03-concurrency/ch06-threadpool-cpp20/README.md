# 第 6 章 线程池 + C++20 并发专题（第三部分 · 并发与多线程）

## 本章目标

- 亲手实现一个可用于真实项目的线程池：固定数量工作线程、投递任务、经 `future` 取回结果、能优雅关闭；
- 分清关闭的两种语义——**排空退出**与**立即退出**——并亲眼看见立即退出时未开工任务的 `future` 如何以 `broken_promise` 收场；
- 专题体验 C++20 的四件新设施（本章专题示例以 `-std=c++20` 单独构建，正文逐处标注）：`jthread`/`stop_token` 协作取消、`counting_semaphore`、`latch`/`barrier`、协程初步；
- 把前五章的零件收束成一个整体：本章的池 = 第 3 章队列 + 第 2 章锁与条件变量 + 本章的 `packaged_task`，阶段项目直接复用。

学完本章，第三部分的知识地图就完整了：线程模型（第 1 章）→ 同步原语（第 2、3 章）→ 原子与内存模型（第 4 章）→ 体检与拆锁（第 5 章）→ 组织成池 + 新标准设施（本章）。

## 1. 为什么要线程池

每来一个任务就 `new` 一条线程，有三个问题：

1. **创建/销毁有成本**。一条线程的栈空间以 MB 计，创建还要走系统调用；座舱里 CAN 信号以毫秒级到达，按信号建线程等于把 CPU 耗在建拆上。
2. **数量不可控**。任务洪峰时线程数随任务数膨胀，几千条线程互相抢核，吞吐反而崩塌——线程不是越多越快（第 2 章的锁开销实测已经见过竞争的代价）。
3. **生命周期散落**。每条线程都要有人负责 join，散落在业务代码里迟早漏（第 1 章的 `bad_01` 就是忘 join 的下场）。

线程池把三件事收拢：**线程预先建好、数量固定、任务排队等待**。业务只管投递任务和取结果，线程的生老病死由池统一管理。这也是为什么从 Java 的 `ExecutorService` 到 Android 的 `HandlerThread` 池化、再到车载中间件的执行器（Executor）概念，本质都是同一个结构。

## 2. 池的骨架：工作队列 + 条件变量 + 关闭协议

`ex01_thread_pool.cpp` 的 `ThreadPool` 拆开看只有三件熟悉的东西：

- **工作队列**：`std::queue<std::function<void()>>`，存放待执行的任务；
- **同步**：一把 `mutex` 保护队列，一个 `condition_variable` 让空闲工作线程睡等——与第 3 章阻塞队列完全同源，谓词是 `stopping_ || !tasks_.empty()`；
- **关闭协议**：`stopping_` 标志 + 叫醒全体 + 逐个 join，这是池与裸队列的唯一本质区别。

工作线程的主循环值得逐行读：等待谓词成立后，**先判队列是否为空**——停止且排空才退出，否则取出队头执行。注意任务是在**解锁之后**执行的：持锁执行会把整个池串行化（第 2 章细粒度锁的教训）。

## 3. 取结果：packaged_task 与 future

裸队列只能"发射后不管"，池要能取结果，通道就是第 1 章见过的 `std::future`，生产端是 `std::packaged_task`：

```cpp
template <typename F>
std::future<typename std::result_of<F()>::type> submit(F&& f) {
    typedef typename std::result_of<F()>::type R;
    std::shared_ptr<std::packaged_task<R()> > task(
        new std::packaged_task<R()>(std::forward<F>(f)));
    std::future<R> fut = task->get_future();
    {
        std::lock_guard<std::mutex> lk(m_);
        if (stopping_) throw std::runtime_error("submit on stopped pool");
        tasks_.push([task] { (*task)(); });
    }
    cv_.notify_one();
    return fut;
}
```

三处细节：

1. **shared_ptr 中转是必经的坑**。`packaged_task` 只可移动不可拷贝，而 `std::function` 要求目标可拷贝——直接把 `packaged_task` 塞进队列编译都过不了。用 `shared_ptr` 包一层再按值捕获进 lambda，是 C++11 线程池的标准解法。
2. **异常会顺着 future 回来**。任务里抛出的异常被 `packaged_task` 捕获存好，在 `fut.get()` 处原样重抛——投递方与执行方隔着队列，异常通道不能丢。
3. **按投递顺序取结果，输出就与调度无关**。主演示投 20 个平方任务、按序 `get()`，无论哪个工作线程先算完，打印恒为 `0 1 4 … 361`、总和恒为 2470。并发程序想要可验收，先学会把不确定性关进笼子。

## 4. 两种关闭语义：排空 vs 立即

关闭不是只有一个动作，语义选错就是线上事故：

- **`shutdown()` 排空退出**：停止接收新任务（再 `submit` 直接抛异常），已入队的任务**全部做完**，工作线程才退出。适合"数据不能丢"的收尾——日志刷盘、信号落库都属于这类。
- **`shutdownNow()` 立即退出**：停止接收，并把队列里**还没开工**的任务直接丢弃，工作线程做完手头这一个就走。适合"再算也没意义"的场景——进程要退、会话已断。

立即退出有个必须亲眼看一遍的后果：被丢弃任务的 `packaged_task` 在队列清空时被销毁，它的 `future` 永远等不到结果，`get()` 会抛出 `std::future_error`，错误码 `broken_promise`。`ex01` 的演示段精确构造了这个场景：1 个工作线程被长任务占住、5 个任务堵在队列里，`shutdownNow` 后——

```
shutdownNow demo: ran = 1, broken futures = 5 (expected 1 and 5)
```

正在执行的长任务**没有**被打断（它靠主线程放行才结束）：线程没有强杀，立即退出也只能"不等排队的"，管不了"正在跑的"。这正是练习 4 要想清楚的问题。

## 5. C++20 专题一：jthread 与 stop_token【C++20】

`ex02_jthread.cpp` 演示 `std::jthread` 对 `std::thread` 的两点改进：

1. **析构自动 join**。作用域结束时析构函数自动 `request_stop()` 再 `join()`——第 1 章"忘 join 就 `terminate`"的坑从语言层被填平，也不用再写 `joinable()` 判断。
2. **内建协作取消**。任务签名以 `std::stop_token` 为首参时，`jthread` 自动把令牌注入；外部 `request_stop()` 只是置位一个标志，任务在循环里轮询 `stop_requested()` 自行决定何时退出。还可以注册 `std::stop_callback`，让停止请求触发一段清理代码。

注意动词：`request_stop` 是**请求**。任务若在死循环里从不看令牌，它永远不会停——协作的另一面是：取消点要由任务作者亲手埋（长循环、阻塞等待处），这不是缺陷，是第 7 节要谈的原则。

## 6. C++20 专题二：counting_semaphore / latch / barrier【C++20】

`ex03_sem_latch_barrier.cpp` 在一个程序里串起三件新原语，顺带与第 3 章的 POSIX 前辈对照：

| 原语 | 语义 | 一次性/可复用 | 第 3 章的对应物 |
|---|---|---|---|
| `counting_semaphore<N>` | 计数许可：`acquire` 取、`release` 还，上限限流 | 可复用 | POSIX `sem_t`（接口回到类型系统，不再裸句柄） |
| `latch` | 倒计数到 0 后全体放行 | **一次性**，不可重置 | 无直接对应（当时用条件变量手搭） |
| `barrier` | 分阶段栅栏：全员到齐才一起进入下一阶段，可带完成回调 | 可复用（自动进入下一期） | 无直接对应 |

实测断言有两条，都是确定性的：

- 6 个线程经 `latch` 同时起跑、抢初值为 3 的信号量，原子计数记录到的**最大并发恰为 3**——限流不是"大概限住"，是可验收的硬顶；
- 两轮工作之间隔着 `barrier`：所有 `phase 0` 日志严格出现在分界线之前、`phase 1` 严格在后。秘诀在完成回调的执行时机——它在**放行之前**由最后到达的线程执行，所以拿它打印分界线，顺序天然成立。

## 7. C++20 专题三：协程一瞥【C++20】

`ex04_coroutine_peek.cpp` 是一个手写的最小生成器：`fib()` 里每次 `co_yield` 都把当前值交出去并**冻结**整个函数（局部变量 `a`、`b` 与执行点都被保存在协程帧里），调用方 `next()` 一推，它从冻结处苏醒继续算下一个。

要建立的心智模型只有一句：**协程 = 可暂停的函数，暂停点由 `co_yield`/`co_await` 标明，状态由编译器生成的帧保存**。它与线程无关——生成器全程跑在调用方线程上，没有任何并发。协程解决的是"异步/迭代逻辑写成顺序代码"的问题（生成器、异步 IO 的状态机），不是"并行"的问题。

C++20 只标准化了语言底座与 promise 协议，`Generator<T>` 这类成品类型要自己拼（成品 `std::generator` 在 C++23 才进标准库）。本章只做一瞥：拼一次 promise、跑通斐波那契前 10 项（`0 1 1 2 3 5 8 13 21 34`），知道底座长什么样即可；协程的深入应用留待后续专题。

## 示例代码：构建、运行与验收

```bash
cmake -S examples -B examples/build && cmake --build examples/build
./examples/build/ex01_thread_pool        # C++11 构建
./examples/build/ex02_jthread            # 以下三个为 C++20 构建（CMake 按目标单独抬标准）
./examples/build/ex03_sem_latch_barrier
./examples/build/ex04_coroutine_peek
bash tests/run_tests.sh                  # 全章验收（含 solutions 与 TSan 复核）
```

CMake 里基线仍是 C++11：`ex01` 按基线构建，`ex02~ex04` 用 `target_compile_features(... cxx_std_20)` 按目标单独抬到 C++20——"基线 + 专题"在本教程里就是这样落地的。（GCC 在 14 之前还要给协程目标补 `-fcoroutines`，CMakeLists 里已按编译器处理。）

本章实测输出（作者环境）：

```
ex01  results: 0 1 4 9 16 25 36 49 64 81 100 121 144 169 196 225 256 289 324 361
      sum of squares 0..19 = 2470 (expected 2470)
      shutdownNow demo: ran = 1, broken futures = 5 (expected 1 and 5)
ex02  worker: stop requested, exiting at tick 7
      main: jthread auto-joined on scope exit
ex04  fib: 0 1 1 2 3 5 8 13 21 34
```

**验证记录（测试先行）**：`tests/run_tests.sh` 先于全部示例写定，首跑在构建阶段判红（passed=0）。实现完成后首轮 9/10——唯一失败的 `ex03` 分段断言，经查是**验收脚本自身的 awk 状态机写错**（示例的原始输出分段完全正确）：判据的 bug 与实现的 bug 要分开定性，修判据后 10/10 全绿、连跑三轮稳定；`ex01` 另经 TSan 构建复核零报告（构建/运行方式同第 5 章的沙箱说明）。

## 动手练习

见 `exercises.md`，共 4 题：优先级/动态扩缩的方案设计（文字）、把第 3 章 `BlockingQueue` 改造成池的工作队列（代码，阶段项目的预演）、用 `jthread` 重写第 1 章生命周期示例（代码）、协作取消为何不能强杀（思考）。代码题参考答案在 `solutions/`。

## 自测清单

- [ ] 能默写线程池的三个组成件，并说清谓词为什么是 `stopping || !empty`
- [ ] 能解释 `packaged_task` 为什么必须经 `shared_ptr` 才能进 `std::function` 队列
- [ ] 任务抛出的异常最终在哪里、以什么形式被投递方拿到
- [ ] 排空退出与立即退出的区别；立即退出后，未开工任务的 `future.get()` 抛什么、错误码是什么
- [ ] `jthread` 相比 `thread` 多做了哪两件事；`request_stop` 之后任务靠什么真正停下来
- [ ] `latch` 与 `barrier` 的一次性/可复用之别；`barrier` 完成回调在放行前还是放行后执行
- [ ] 协程的冻结/恢复与线程切换的区别：生成器跑在哪个线程上
- [ ] 自己的构建里，哪个示例是 C++11、哪些是 C++20，CMake 是怎么按目标区分的

## 本章小结与下一章预告

本章把零件组装成了机器：线程池 = 队列 + 同步 + 关闭协议 + future 通道；又把 C++20 的四件新设施各用一个最小场景验明正身。至此第三部分的六章正课全部结束。

下一站是**阶段项目 project-taskqueue-logger**：把第 3 章定型的 `BlockingQueue<T>`、本章的 `ThreadPool` 与一个异步 `Logger` 组装成可复用的并发基础设施——验收标准包括万级并发日志落盘不丢行、双 sanitizer 零报告。这套接口将在第六部分毕业项目（座舱信号网关）中原样复用。
