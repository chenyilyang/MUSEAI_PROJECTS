# 第 5 章 并发安全与死锁：用 TSan 体检，用顺序拆锁（第三部分 · 并发与多线程）

## 本章目标

学完这一章，你应该能做到：

- 用 `-fsanitize=thread` 构建并运行程序，读懂 ThreadSanitizer 报告的每个关键部分；
- 说清 TSan 的判定标准（无同步的并发访问对），以及"TSan 干净 ≠ 逻辑正确"的边界；
- 亲手复现一次教科书死锁，用死锁四条件对号入座，并知道现场怎么诊断；
- 用全局锁顺序与 `std::lock` 两种方法拆解死锁，形成自己的并发自查清单。

**前置知识**：第 1~4 章全部。本章是前四章的体检与收束：例子全是旧账——第 1 章的计数器、第 2 章的双锁。

**本章文件**：示例在 `examples/`（2 个修复/对照 + 2 个反例），验收测试在 `tests/run_tests.sh`（测试先行；其中 TSan 构建由脚本现场以编译开关完成），练习在 `exercises.md`，参考答案在 `solutions/`。

---

## 1. ThreadSanitizer：给并发访问装监控

TSan 是编译器插桩工具：加一个开关重新构建，它就在每次内存访问与每次同步操作上做记录，运行时比对"两个线程碰同一地址、至少一方是写、且中间没有同步关系"——命中即报告。它不需要你的程序真的算错：**它抓的是结构，不是结果**。

```bash
g++ -std=c++11 -g -fsanitize=thread -pthread bad_01_race.cpp -o tsan_race
./tsan_race
```

代价要心里有数：开了 TSan 的程序慢约 5~15 倍、吃内存数倍，所以它住在测试/CI 里，不进生产构建。`-g` 不能省——没有调试信息，报告里的行号就无从谈起。它与第二部分用过的 ASan 分工不同：ASan 查内存错误（越界、释放后使用），TSan 查并发错误，两个都该进你的工具箱。

> 环境注记：某些受限沙箱/容器里 TSan 会因地址布局报 `FATAL: ThreadSanitizer: unexpected memory mapping` 随机失败。本章验收脚本的对策是 `-no-pie` 构建 + `setarch -R` 运行（作者沙箱实测：不加时约半数运行 FATAL，加后稳定出报告）。在你自己的开发机上通常直接运行即可；若遇到同样的 FATAL，这两招值得一试。

## 2. 读报告实战：第 1 章的账被点名

`bad_01_race.cpp` 就是第 1 章 ex05 的共享计数原样：普通构建跑完只是数字不准，悄无声息。TSan 构建下，它开口了（实测报告节选，地址每次不同）：

```
WARNING: ThreadSanitizer: data race (pid=3328)
  Read of size 8 at 0x0000004070c8 by thread T2:
    #0 bump examples/bad_01_race.cpp:16
    ...
  Previous write of size 8 at 0x0000004070c8 by thread T1:
    #0 bump examples/bad_01_race.cpp:16
    ...
  Location is global 'g_counter' of size 8 at 0x0000004070c8
  Thread T2 (tid=3331, running) created by main thread at:
    ...
    #2 main examples/bad_01_race.cpp:21
SUMMARY: ThreadSanitizer: data race examples/bad_01_race.cpp:16 in bump
```

逐块翻译：

- **第一块 vs 第二块**：一对冲突访问——T2 在此**读**、T1 此前**写**，同一行（`bad_01_race.cpp:16`，就是 `++g_counter`）、同一地址；
- **Location 行**：点名变量本体——全局变量 `g_counter`，8 字节；
- **created by 块**：两个线程的出生证明（主线程在第 21 行创建的它们），帮你把"哪两段代码在打架"对上号；
- **SUMMARY**：一句话归档，CI 里通常 grep 这一行计数。

对照组 `ex02_tsan_clean_counter.cpp` 是第 2 章的 mutex 版：同一份活，TSan 下零报告、退出码 0、结果精确。验收正是按这一"脏"一"净"判的。记住 TSan 的两个边界：它只报**实际运行中执行到**的路径（测试覆盖不到的代码它看不见）；它报的是语言层面的数据竞争，**逻辑层面的竞态它管不了**——比如第 4 章练习里"检查再扣座"的两步各自合法加锁、组合照样超卖，TSan 会判它干净。工具与纪律，缺一不可。

## 3. 死锁复现：四条件对号入座

`bad_02_deadlock.cpp` 把死锁做成必现：线程 1 先锁 A 再够 B，线程 2 先锁 B 再够 A，中间各睡 100ms 保证双方都先攥住第一把。运行结果只有两行输出，然后永久安静（验收用 timeout 收尸判过）。教科书的**死锁四条件**在此一一对上：

| 条件 | 本例对应 |
|---|---|
| 互斥 | mutex 同一时刻只容一人 |
| 持有并等待 | 攥着 A 等 B（不先放手） |
| 不可剥夺 | 没人能把对方的锁抢走 |
| 循环等待 | 1 等 2 的 B，2 等 1 的 A，成环 |

四条件缺一，死锁即破——下一节的两种修法各破其一。

现场诊断的通用手法是抓栈：程序挂住后 `gdb -p <pid>` 接入，再 `thread apply all bt` 看每个线程停在哪一行——两个线程互等的环会直接画在你面前。（作者注记：我的沙箱禁止 ptrace，未能在此实测抓栈；这组命令请你在自己的机器上对着 bad_02 亲手跑一遍，练习 3 之后正合适。）

## 4. 两种拆法（ex01 与练习 3）

`ex01_deadlock_fixed.cpp` 把场景升级为双向转账（每账户一把锁，天生反序地形），用的是第一种拆法：

- **`std::lock`（破"持有并等待")**：`std::lock(m1, m2)` 以避免死锁的算法一次性锁多把——要么全拿到，要么一把都不占着等。锁完再用 `lock_guard` + `adopt_lock` 接管释放，RAII 不丢。验收：双向各转 1000 次后两账户各回 1000、总额 2000 守恒。
- **全局锁顺序（破"循环等待")**：给所有锁排一个全局次序（如按对象地址），任何人必须按序拿锁。练习 3 里你会亲手写一遍，并与 std::lock 版并排跑。

顺带泼一盆冷水：`try_lock` + 失败重试不是免费的午餐——两个线程可能同步地反复抢、反复退（**活锁**：没人睡死，但也没人前进）。另外两条纪律写进清单：持锁时不要调用不可控的外部代码（回调、虚函数、别人的模块——你不知道它会再锁什么）；锁嵌套层数越深，排序越难守，能不嵌套就不嵌套。

## 5. 并发自查清单（收束前四章）

写任何并发代码前过一遍：

- [ ] 每份共享可变数据，是否指派了唯一的那把锁（或明确的原子方案）？
- [ ] 锁是否全部 RAII 化（lock_guard/unique_lock），无裸 lock/unlock？
- [ ] 临界区是否只圈共享操作，慢活（IO、计算、回调）是否已挪到锁外？
- [ ] 条件变量是否一律谓词式 wait、先改状态再 notify、收尾用 notify_all？
- [ ] 多把锁同拿时，是否走了全局顺序或 std::lock？
- [ ] 原子变量的内存序是否为"捎带的可见性"想清楚了（计数 relaxed、发布-订阅 acq/rel、拿不准 seq_cst)？
- [ ] 新代码是否在 CI 里过一轮 TSan 构建？

---

## 示例代码：构建、运行与验收

```bash
cd examples
cmake -S . -B build && cmake --build build
./build/bad_01_race
g++ -std=c++11 -g -fsanitize=thread -pthread bad_01_race.cpp -o /tmp/tsan_race && /tmp/tsan_race
timeout 5 ./build/bad_02_deadlock    # 预期挂死，退出码 124
./build/ex01_deadlock_fixed
./build/ex02_tsan_clean_counter

bash tests/run_tests.sh
```

核对要点（与验收断言对应）：

- `bad_01`：普通构建跑完有结果行；TSan 构建必须输出含 `SUMMARY: ThreadSanitizer: data race` 的报告；
- `bad_02`：两行 holding 日志齐全且进程挂死（timeout 124）；
- `ex01`：`A = 1000, B = 1000, total = 2000 (expected)`；
- `ex02`：TSan 构建零报告、退出码 0、计数精确。

> 验证记录（测试先行）：验收脚本先行写定，首跑构建判红（passed=0）；实现后全绿（passed=6），复跑稳定，`-std=c++11 -Wall -Wextra` 零警告。本章的真实波折记录在案：TSan 在作者沙箱里不加干预时约半数运行报 `FATAL: unexpected memory mapping`，以 `-no-pie` 构建 + `setarch -R` 运行后 4/4 稳定，验收脚本已固化此对策并注明普通开发机无需如此；gdb 抓栈因沙箱禁 ptrace 未能实测，正文已如实标注。环境 Ubuntu 24.04 / GCC 13.3，日期 2026-10-23。

## 动手练习

见本章 `exercises.md`（5 道题；参考答案在 `solutions/`）。

## 自测清单

- [ ] 我能独立给一个程序做 TSan 构建并定位报告中的冲突双方、变量与创建点
- [ ] 我能说清 TSan 的判定标准与两个边界（未执行路径、逻辑竞态）
- [ ] 我能默写死锁四条件，并把任一死锁现场对号入座
- [ ] 我能用全局顺序与 std::lock 两种方法改写一段反序加锁代码
- [ ] 我知道活锁是什么，以及为什么 try_lock 重试不是死锁的万能解

## 本章小结与下一章预告

体检与拆锁都练过了：工具负责把结构问题摆到你面前，纪律负责不让它们再生。**下一章（线程池 + C++20 并发专题）**是本部分收官章：先用 C++11 把线程池写出来（任务队列、future 取结果、优雅关闭），再专题体验 C++20 的 jthread/stop_token、counting_semaphore、latch/barrier 与协程一瞥——之后便是阶段项目：把任务队列与日志库定型交付。
