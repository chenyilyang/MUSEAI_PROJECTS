# 第 5 章 信号（第二部分 · Linux 开发起步）

## 本章目标

学完这一章，你应该能做到：

- 说清信号是什么、默认处置有哪几种，知道为什么大多数服务进程都要自己接管关键信号；
- 用 sigaction 正确安装处理函数，并解释它比 signal() 多控制了什么；
- 背得出**异步信号安全**的红线：处理函数里只许对 `volatile sig_atomic_t` 标志做读写，其余事一律回主循环；
- 用 SIGCHLD + `waitpid(-1, WNOHANG)` 排干循环系统性回收子进程，永不留僵尸，并会用 ECHILD 自证；
- 用 setitimer + SIGALRM 给程序装上周期节拍。

**前置知识**：第二部分第 3、4 章（wait 解码、子进程 _exit 纪律、守护进程里见过的最简版 SIGTERM 标志位处理——本章把那套做法的原理讲透）。本章起改为测试先行方式产出：`tests/` 里的验收脚本先于示例写定，你改示例或自己重写时都能用它验收。

**本章文件**：示例在 `examples/`（3 个），验收测试在 `tests/run_tests.sh`，练习在 `exercises.md`，参考答案在 `solutions/`。

---

## 1. 信号是什么

信号是内核给进程递的"异步通知条"：某个事件发生了（子进程结束、定时器到点、用户按了 Ctrl+C、有人执行了 kill），进程的正常执行流会被**随时打断**，先去执行为该信号登记的处理函数，回来再接着跑。每个信号有编号与默认处置——终止、终止并 core dump、忽略、暂停/继续，五花八门；除了 SIGKILL(9) 与 SIGSTOP(19) 不可接管外，你都可以为信号改派自己的处理函数。

要先建立的直觉是：信号处理函数（handler）运行在一个"借来的上下文"里——它打断的代码可能正执行到任何一行、任何一把锁的中间。所以 handler 的世界规则与平时完全不同，这是本章的题眼，见第 3 节。

常用信号速查（本章用到的）：SIGINT(2，终端中断）、SIGTERM(15，请求终止）、SIGCHLD(17，子进程状态变化）、SIGALRM(14，定时器到点）。

## 2. 两种装法：signal() 与 sigaction()

`ex01_signal_vs_sigaction.cpp` 用同一场景演示两种装法——进程等 SIGINT，handler 只把计数器 +1，主循环数满 2 次收尾：

```bash
./ex01_signal_vs_sigaction signal     # 或 sigaction
# 脚本发两次 SIGINT，程序打印：
mode = sigaction, interrupts = 2, bye
```

挨了第一下 SIGINT 却没死，是"默认处置（终止）已被你的 handler 替换"的直接证据。两种装法外部行为一致，差别在**可控性**：

- `signal()` 是历史简装：一行搞定，但处理期间是否屏蔽同种信号、被打断的系统调用是否自动重启（SA_RESTART 语义），在不同年代的 Unix 上口径不一，移植即埋雷；
- `sigaction()` 是正装：处理函数、**处理期间额外屏蔽的信号集**（sa_mask）、行为标志（sa_flags，如 SA_RESTART）全部由你显式填写。注意一个默认项：handler 执行期间，**正在处理的那种信号本身会被自动屏蔽**，避免 handler 被自己重入。

结论直接记：**新代码一律 sigaction**，signal() 只用来读老代码时认脸。

## 3. 异步信号安全：本章的题眼

为什么 handler 里只能碰 `volatile sig_atomic_t`？设想主循环正 printf 到一半（stdio 内部锁已拿下、缓冲结构改了一半），信号到达，handler 里又调 printf——它会去拿同一把锁：**死锁**，或者踩着改了一半的结构前进：**崩溃/数据损坏**。malloc、exit、C++ 的大多数设施全都有类似的内部状态，都在禁区里。这类"可能在信号打断点上被重入"的函数不具备**异步信号安全**（async-signal-safe）。

红线因此非常干脆：

- handler 里只许：给 `volatile sig_atomic_t` 变量**赋值或自增**（这个类型的读写被保证是原子的、单步完成的），以及调用 POSIX 明确列名的异步信号安全函数（如 `write`、`_exit`——注意是 `_exit` 不是 `exit`，后者要冲 stdio 缓冲，不安全）；
- 其余一切——打日志、回收子进程、改数据结构——全部**回主循环做**：handler 只负责置标志/计数，主循环轮询到变化再动手。ex01 的计数、ex02 的回收、ex03 的打印、上一章守护进程的收尾，全是这一条架构。

`volatile` 也不可省：没有它，编译器可能把"循环里反复读的变量"优化成只读一次寄存器，主循环就永远等不到 handler 改的新值。

## 4. SIGCHLD 系统性回收：排干循环

第 3 章练习 5 埋的伏笔在这里兑现。机制：**任一子进程状态变化（结束），内核向父进程发 SIGCHLD**。但标准信号（非实时信号）**不排队**：已有同种信号待处理时，再来的同种信号会被合并丢弃——5 个子进程接连结束，你可能只收到 1 个 SIGCHLD。若 handler 或主循环"收到一个信号收一个孩子"，漏收的孩子就成了永久僵尸。

正确姿势是 `ex02_sigchld_reap.cpp` 的三段：handler 只置标志 → 主循环见标志后执行**排干循环**

```cpp
while ((p = waitpid(-1, &status, WNOHANG)) > 0) { /* 逐个登记 */ }
```

一口气收到"暂时没有已结束的子进程"为止；计数满额后，再做一次 waitpid 验证——返回 -1 且 `errno == ECHILD`（"你已经没有任何子进程了"），这是无僵尸的**程序化证明**，比 ps 里肉眼找 Z 更可靠。验收测试正是按这套断言写的：5 个子进程退出码 11..15 必须全数回收、ECHILD 证明必须出现。

## 5. 定时器信号：setitimer + SIGALRM

周期任务不靠 sleep 数羊：`setitimer(ITIMER_REAL, ...)` 让内核按真实时间周期性递送 SIGALRM（另有 ITIMER_VIRTUAL/PROF 分别按进程用户态时间/总 CPU 时间计，本章用 REAL）。`ex03_timer_tick.cpp` 每 500ms 一拍，handler 只累加计数，主循环在 2、4 拍各打一行摘要、满 6 拍先停表再报总数：

```
tick summary: ticks = 2
tick summary: ticks = 4
total ticks = 6
```

注意主循环报总数前先把定时器关掉——又是一处"顺序即正确性"的细节：不停表，计数会在打印前后继续变，总数就成了不可复现的随机数（验收测试断言恰好为 6，这正是它要拦的毛病）。

---

## 示例代码：构建、运行与验收

```bash
cd examples
cmake -S . -B build && cmake --build build
./build/ex01_signal_vs_sigaction sigaction    # 另开终端 kill -INT <pid> 两次
./build/ex02_sigchld_reap
./build/ex03_timer_tick                       # 约 3 秒自停

# 一键验收（先于实现写定的就是它）：
bash tests/run_tests.sh
```

核对要点（与 `tests/run_tests.sh` 的断言一一对应）：

- `ex01` 两种装法下行为一致：第一下 SIGINT 后进程仍活着，第二下后打印 `interrupts = 2` 并以 0 退出；
- `ex02`：5 个子进程全部被回收（退出码集合恰为 11..15)，末行出现 `no zombies left (ECHILD)`；
- `ex03`:2、4 拍摘要各一行，总数恰为 6。

> 验证记录（本章起改为测试先行）：验收脚本 `tests/run_tests.sh` 先于示例实现写定，首跑时因实现尚不存在在构建阶段判红（passed=0）；实现完成后全绿（passed=5，含两种装法各一项），并完整复跑一次仍为全绿；构建为 `-std=c++11 -Wall -Wextra` 零警告。环境 Ubuntu 24.04 / GCC 13.3，日期 2026-10-23。

## 动手练习

见本章 `exercises.md`（5 道题；参考答案在 `solutions/`）。

## 自测清单

- [ ] 我能解释信号为什么是"异步"的，以及默认处置可以被谁替换、哪两个信号不能接管
- [ ] 我写得出 sigaction 的安装代码，并能说清 sa_mask 与 SA_RESTART 各管什么
- [ ] 我背得出异步信号安全红线，并能举例说明 printf 在 handler 里为什么是雷
- [ ] 我知道标准信号会合并，并因此坚持 SIGCHLD 回收用 WNOHANG 排干循环、用 ECHILD 自证无僵尸
- [ ] 我会用 setitimer 装周期节拍，且记得报总数前先停表

## 本章小结与下一章预告

第二部分的最后一块知识拼图到位：信号的接管、红线与两个实战套路（回收、节拍）。**下一项是阶段项目 project-log-collector**：fork 多个 worker 子进程经管道汇写日志、父进程统一打时间戳落盘、SIGCHLD 无僵尸回收、SIGTERM 优雅收尾——把第 2 章的管道、第 3 章的进程、第 5 章的信号一次总装。做完它，第二部分就收官了。
