# LearnLinux 第三部分（并发与多线程）逐章细化计划

> 依据：主计划 Task 9 路线图（章节标题与顺序以路线图为准）。生产规则沿用既定约定：逐章确认制；自第二部分第 5 章起的**测试先行**（每章 `tests/run_tests.sh` 先于示例写定并跑红，再实现转绿，脚本随章入库）；C++11 为工作基线（C++20 并发设施集中在第 6 章专题，届时以 `-std=c++20` 单独构建并在正文标注）；并发章节的示例除常规断言外，凡涉及数据竞争/死锁的，另以 TSan 构建复核（第 5 章起为硬性要求，第 1 章的竞争初瞥先做行为断言、TSan 留待第 5 章回访）。

## Task 9.1 第 1 章 线程模型：创建、参数与生命周期

- **目录**：`part03-concurrency/ch01-thread-model/`
- **目标**：会用 std::thread 与 pthread 两套 API 创建/回收线程；分清参数传递的拷贝与引用；守住生命周期纪律（joinable 的线程对象析构即 terminate）。
- **知识点**：进程与线程的关系（共享地址空间、各自栈与寄存器）；std::thread 创建与 thread id；参数传递的本质是拷贝（decay-copy），引用须 std::ref；join 与 detach 的语义与代价；joinable 状态与 std::terminate 陷阱；线程函数的返回值通道（共享变量/指针 vs pthread 的 void* 返回）；竞争初瞥（共享计数丢失更新）为下一章埋动机。
- **示例清单**：
  - `ex01_thread_basics.cpp`：4 个 worker 各算一段阶梯和写入专属槽位，主线程 join 后按序打印；核对：槽位值 55/210/465/820 精确，4 个 worker 的 thread id 互不相同且都不同于主线程。
  - `ex02_args_and_ref.cpp`：值参、std::ref 引用参、指针参三式对照；核对：值参改不动原值（仍为 1），引用参改得动（变为 99），指针参改得动（变为 7）。
  - `ex03_lifecycle.cpp`：joinable 状态在 join 前后为 1→0；detach 的线程在主线程存活窗口内独立完成计数；核对：三行断言齐全。
  - `bad_01_forgot_join.cpp`（反例）：线程对象未 join/detach 即析构；核对：进程以 SIGABRT 结束且输出含 `terminate called`。
  - `ex04_pthread.cpp`：pthread_create/pthread_join，工作函数经 void* 返回堆上结果；核对：主线程取回 42。
  - `ex05_race_glimpse.cpp`：4 线程共享一个普通 int 各加 250000 次 vs 各自独立计数；核对：独立计数总和恰为 1000000，共享计数 ≤ 1000000（实测值写入正文并注明每次不同）。
- **练习方向**：生命周期判断（哪些写法会 terminate）；读代码找悬垂引用（detach + 栈变量）；写代码：多线程分段求和与单线程对照；写代码：pthread 版带返回值计算；思考：detach 的正当用途与代价。

## Task 9.2 第 2 章 同步原语（一）：互斥与 RAII 锁

- **目录**：`part03-concurrency/ch02-sync-primitives-1/`
- **目标**：用 mutex 把第 1 章丢更新的计数器修到精确；掌握 lock_guard/unique_lock 的 RAII 纪律与取舍；会用读写锁处理读多写少。
- **知识点**：互斥锁语义与临界区粒度；std::mutex 与死等；lock_guard（简单、不可移动）与 unique_lock（可延迟/可转移、配条件变量）；std::lock 与 try_lock 防死锁一瞥；shared_mutex/shared_lock 读写锁（C++17 设施，本章以 -std=c++17 构建该示例并标注，或以 pthread_rwlock 实现——细化时二选一，倾向 pthread_rwlock 以守 C++11 基线，shared_mutex 在 C++17 专题回访）；锁的开销实测（无锁/粗锁/细锁三档计时对比）。
- **示例清单**：
  - `ex01_mutex_counter.cpp`：第 1 章的共享计数加 mutex；核对：总数恰为 1000000。
  - `ex02_raii_locks.cpp`：lock_guard 与 unique_lock 两种写法保护同一临界区，含异常路径下锁仍释放的演示；核对：结果精确且异常被外层捕获、程序正常收尾。
  - `ex03_rwlock.cpp`：pthread_rwlock，多读者并发读、写者排他改一个配置值；核对：读者从不见中间态，最终值正确。
  - `ex04_lock_cost.cpp`：三种策略跑同一负载打印耗时（只断言量级关系，不断言具体毫秒）。
- **练习方向**：找临界区划分错误；把裸 lock/unlock 改 RAII；写代码：线程安全计数器类；思考：锁粒度与吞吐的权衡。

## Task 9.3 第 3 章 同步原语（二）：条件变量与生产者消费者

- **目录**：`part03-concurrency/ch03-sync-primitives-2/`
- **目标**：用 condition_variable 写出无忙等的阻塞队列雏形；理解虚假唤醒与谓词写法的必然性；认识 POSIX 信号量并预告 C++20 counting_semaphore。
- **知识点**：条件变量 wait/notify_one/notify_all；wait 必须配谓词循环（虚假唤醒）；生产者-消费者与有界队列；unique_lock 为何是条件变量的搭档；POSIX sem_init/sem_wait/sem_post 计数信号量；丢失唤醒的成因。
- **示例清单**：
  - `ex01_condvar_handoff.cpp`：一对一信号交接（worker 等主线程发令才开跑）；核对：顺序日志严格有序。
  - `ex02_bounded_queue.cpp`：有界队列（容量 4）+ 2 生产者 2 消费者，总量守恒；核对：消费总数与总和精确，队列长度从未越界（内部断言）。
  - `ex03_posix_sem.cpp`：POSIX 信号量实现同一交接/计数场景；核对：与条件变量版结果一致。
  - `bad_01_lost_wakeup.cpp`（反例）：先 notify 后 wait 的丢失唤醒时序，程序挂住；核对：以 timeout 运行，预期超时（退出码 124）证明其挂死——反例的"通过"就是挂住。
- **练习方向**：给 wait 补谓词；读代码找丢失唤醒；写代码：阻塞队列类（为阶段项目打底）；思考：notify_one 与 notify_all 的选型。

## Task 9.4 第 4 章 原子操作与内存模型入门

- **目录**：`part03-concurrency/ch04-atomics-memory-model/`
- **目标**：会用 std::atomic 做无锁计数与标志；理解 happens-before 与内存序的入门直觉（relaxed 与 acquire/release 的分工）；见过无锁队列的思想与 ABA 问题。
- **知识点**：atomic 的 load/store/fetch_add 与 CAS（compare_exchange）；原子标志做自旋锁及其代价；内存序谱系（relaxed / acquire / release / seq_cst）与"发布-订阅"模式；数据竞争与未定义行为的界限（与第 5 章 TSan 呼应）；无锁栈/队列结构思想、ABA 与 hazard pointer 概念一瞥（只讲思想，不做生产级实现）。
- **示例清单**：
  - `ex01_atomic_counter.cpp`：第 1 章计数改 atomic；核对：恰为 1000000，且与 mutex 版做耗时对照（只断言量级）。
  - `ex02_spinlock.cpp`：atomic_flag 自旋锁保护临界区；核对：计数精确，并注明其适用场景（极短临界区）。
  - `ex03_publish_subscribe.cpp`：release 写数据+标志、acquire 读标志后读数据；核对：读者要么读到完整新数据，要么读到旧数据，永不读到一半。
  - `ex04_lockfree_stack.cpp`：CAS 循环实现的无锁栈，push/pop 总量守恒；核对：元素总和精确（单线程回收策略下运行，内存回收问题在正文点明）。
- **练习方向**：给场景选内存序并说明理由；读代码找把 atomic 误当万能同步的错误；写代码：原子版一次性初始化标志；思考：无锁不等于无等待/高性能的情形。

## Task 9.5 第 5 章 并发安全与死锁：TSan 实测与反例章

- **目录**：`part03-concurrency/ch05-thread-safety-deadlock/`
- **目标**：会用 ThreadSanitizer 定位数据竞争并读懂报告；能复现并拆解典型死锁；形成并发代码的自查清单。
- **知识点**：TSan 工作原理一瞥（影子内存与向量时钟，只讲到能读懂报告）；报告结构（竞争双方、栈、锁集）；死锁四条件与锁顺序纪律；锁超时/try_lock 的局限；第 1 章竞争示例的 TSan 回访。
- **示例清单**：
  - `bad_01_race.cpp`（反例）：第 1 章共享计数原样；核对：TSan 构建运行必须报 `data race` 并指出两处访问位置，普通构建则悄无声息——对照即教材。
  - `bad_02_deadlock.cpp`（反例）：两把锁反序加锁；核对：timeout 内挂死（退出码 124），并附 gdb attach 看双线程栈的方法演示（脚本可选做）。
  - `ex01_deadlock_fixed.cpp`：同场景用全局锁顺序（或 std::lock）修复；核对：正常完成、结果精确。
  - `ex02_tsan_clean_counter.cpp`：mutex 版计数在 TSan 下零报告收尾；核对：无 `ThreadSanitizer` 字样输出、退出码 0。
- **练习方向**：读一份 TSan 报告定位竞争点；给多锁场景排全局顺序；写代码：把一段有竞争的代码修到 TSan 干净；思考：TSan 干净不等于逻辑正确的边界。

## Task 9.6 第 6 章 线程池 + C++20 并发专题

- **目录**：`part03-concurrency/ch06-threadpool-cpp20/`
- **目标**：实现一个可投递任务、取 future 结果、能优雅关闭的线程池；专题体验 C++20 的 jthread/stop_token、counting_semaphore、latch/barrier 与协程初步（本章专题部分以 `-std=c++20` 构建并显式标注）。
- **知识点**：线程池结构（工作队列 + 条件变量 + 关闭协议）；packaged_task/future 取结果；关闭的两种语义（排空退出 vs 立即退出）；jthread 的自动 join 与 stop_token 协作取消；counting_semaphore 与 latch/barrier 的同步点语义；协程 co_await 一瞥（只做最小演示，深入留待后续专题）。
- **示例清单**：
  - `ex01_thread_pool.cpp`（基线 C++11）：固定 4 线程的池，投递 20 个任务经 future 取回平方值；核对：结果集合精确、优雅关闭后全部线程退出。
  - `ex02_jthread.cpp`（C++20）：jthread + stop_token 的可取消循环任务；核对：请求停止后限时内退出，无需手动 join。
  - `ex03_sem_latch_barrier.cpp`（C++20）：counting_semaphore 限流 + barrier 分阶段推进；核对：阶段日志严格分段。
  - `ex04_coroutine_peek.cpp`（C++20）：最小生成器协程产出斐波那契序列；核对：前 10 项精确。
- **练习方向**：给线程池加优先级或动态扩缩的方案设计（文字）；把第 3 章阻塞队列改造成池的工作队列；写代码：用 jthread 重写第 1 章生命周期示例并对比；思考：stop_token 协作取消为何不能强杀线程。

## Task 9.7 阶段项目：线程安全任务队列 + 日志库

- **目录**：`part03-concurrency/project-taskqueue-logger/`
- **形态**：任务书 + 骨架 + 验收脚本（先行）+ 参考实现，与第二部分阶段项目同构。
- **规格要点**：
  - `BlockingQueue<T>`：第 3 章阻塞队列定型为类模板——push/pop、close 后排空语义、容量可选（无界/有界）。
  - `Logger`：异步日志库——前端线程只把日志塞进队列，后端单线程统一格式化落盘；级别过滤（DEBUG/INFO/WARN/ERROR）；进程退出前 flush 并关闭，保证尾部日志不丢。**接口在此定型，第六部分毕业项目直接复用**（类名、方法名以任务书为准）。
  - `ThreadPool`：复用第 6 章实现，验收其任务结果正确性与优雅关闭。
- **验收要点**：多生产者并发写日志 10000 条，落盘行数精确、每行格式完整不交错、按级别过滤计数正确；队列在 close 后 pop 能取完存量并收到结束信号；全程 ASan+UBSan 与 TSan 双构建零报告；骨架版（同步直写、无关闭协议）必须在"尾部不丢"或 TSan 项上被判下。
- **与毕业项目的关系**：网关的内部信号分发队列与日志即用本项目的 BlockingQueue 与 Logger，不再另写。

## 节奏

严格逐章确认：每章交付后停下等学习者确认；第 1 章随本细化一并交付请学习者过目。
