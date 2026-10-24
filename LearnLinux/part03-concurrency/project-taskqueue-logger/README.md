# 阶段项目：线程安全任务队列 + 异步日志库（TaskQueue + Logger）（第三部分）

## 项目目标

把第三部分的零件组装成一套**可复用的并发基础设施**，共三个头文件：

- `blocking_queue.h` —— 线程安全阻塞队列 `BlockingQueue<T>`（第 3 章接口的定型版）；
- `logger.h` —— 异步日志库 `Logger`：前端只入队、后端单线程落盘，带级别过滤与不丢尾的关闭协议；
- `thread_pool.h` —— 第 6 章线程池的定型版，建在 `BlockingQueue` 之上。

这三个接口在本文**定型**：类名、方法名、语义以本文为准，第六部分毕业项目（座舱信号网关）将原样复用，不再另写。

## 背景与交付形态

目录结构与第二部分阶段项目同构：

- `starter/`：可编译的骨架。接口齐全，但关键语义留了 TODO（队列不阻塞、日志无过滤、`stop()` 不排空等），验收**不过**是设计好的——你的任务就是按本文规格把 TODO 补齐；
- `solutions/`：参考实现（验收全过 + 双 sanitizer 干净），做完再对照；
- `tests/run_tests.sh`：验收脚本，先于实现写定。三个验收程序（`queue_check` / `logger_stress` / `pool_check`）在 starter 与 solutions 里是同一份，只链接各自的头文件实现。

## 接口规格（定型）

### BlockingQueue<T>（`blocking_queue.h`）

```cpp
template <typename T>
class BlockingQueue {
public:
    explicit BlockingQueue(std::size_t capacity = 0);  // 0 = 无界
    void push(const T& v);        // 有界且满时阻塞；已关闭时抛 std::runtime_error
    bool pop(T& out);             // 阻塞等待；返回 false 仅当"已关闭且已取空"
    void close();                 // 关门：唤醒全部等待者（生产者与消费者都要）
    std::size_t size() const;
    bool closed() const;
};
```

语义要点：

- **排空语义**：`close()` 之后，队列里的存量仍可被 `pop` 取完，取空后 `pop` 才返回 `false`——这个 `false` 是消费者的结束信号，全项目只有这一种收尾方式；
- **有界阻塞**：容量非 0 时，满队列上的 `push` 必须真阻塞（睡在条件变量上），直到有元素被取走或队列被关闭；被关闭唤醒的 `push` 以 `std::runtime_error` 收场，不许把元素塞进已关门的队列；
- 两个条件变量分工（notEmpty / notFull）还是一个条件变量 + 全谓词，任选，但谓词必须写全、虚假唤醒必须免疫（第 3 章的纪律）。

### Logger（`logger.h`）

```cpp
enum class LogLevel { DEBUG = 0, INFO = 1, WARN = 2, ERROR = 3 };

class Logger {
public:
    explicit Logger(const std::string& path, LogLevel minLevel = LogLevel::DEBUG);
    ~Logger();                                   // 未 stop 过则自动 stop
    void log(LogLevel level, const std::string& msg);
    void debug(const std::string& msg);
    void info(const std::string& msg);
    void warn(const std::string& msg);
    void error(const std::string& msg);
    void setLevel(LogLevel level);
    LogLevel level() const;
    void stop();                                 // 排空 + flush + 回收线程；幂等
};
```

语义要点：

- 构造即打开文件（失败抛 `std::runtime_error`）并启动后端线程；前端 `log()` 只做**级别过滤 → 取时间戳 → 入队**三件事，格式化与写盘全在后端单线程完成，业务线程永不被磁盘 IO 堵住；
- 低于 `minLevel` 的日志在入队前丢弃；`stop()` 之后再 `log()` 的消息**静默丢弃**。这里选丢弃不选抛异常，是有意的：进程收尾阶段各模块析构顺序交错，"日志器已停"不该变成业务代码的异常源（对比：`BlockingQueue::push` 面对的是基础设施内部调用，选抛异常让错误显形——同一决策，两种答案，取决于调用者是谁）；
- `stop()` 的契约是**不丢尾**：调用返回前，已入队的每一条日志都必须已经落盘并 flush，后端线程已经 join；
- 落盘格式定型：`[YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] msg`，时间戳取 `log()` 调用时刻（不是写盘时刻）。单后端写盘 ⇒ 整行天然不交错（第 5 章的结论在这里变现：交错问题靠"单写者"从结构上消灭，而不是靠一把大锁）。

### ThreadPool（`thread_pool.h`）

接口与第 6 章 `ex01` 一致：`ThreadPool(std::size_t workers)`、`submit(F) -> std::future<...>`、`shutdown()`（排空退出）、`shutdownNow()`（立即退出：排队任务被抛弃、其 `future` 以 `broken_promise` 收场、正在执行的跑完）、析构兜底 `shutdown()`。本项目的池必须建立在 `BlockingQueue` 之上——`shutdown()` 就是队列的 `close()`，`shutdownNow()` 需要你自己想一个在该结构上成立的做法（参考实现用的是"抛弃标志 + 取出即丢"，不是唯一解）。

## 程序规格（验收程序契约）

三个验收程序已给定（starter 与 solutions 各一份，内容相同），你只需实现头文件：

- `queue_check`：无界队列 1..100 入队后 `close()`，消费者须取全（count=100, sum=5050）并收到结束信号；有界（容量 2）队列装满后第三次 `push` 须在消费者取走一个之前保持阻塞，之后顺序仍为 1 2 3；
- `logger_stress <路径> <生产者数> <每人条数>`：每个生产者按 `seq%5` 循环打 DEBUG/INFO/WARN/ERROR/INFO（消息体 `p<P> seq <S>`），Logger 最低级别 INFO。验收跑 4×2500：落盘必须恰好 **8000 行**（DEBUG 2000 条被过滤），级别计数 INFO=4000 / WARN=2000 / ERROR=2000，逐行格式合规、每条应出现的消息恰好出现一次（`tests/verify_log.py` 核对）；
- `pool_check`：20 个平方任务总和 2470；投 10 个慢任务后立即 `shutdown()`，返回时已完成数必须为 10（排空语义）。

## 硬性约束

- C++11 基线，头文件自包含，`-Wall -Wextra -Werror` 零警告；
- 只用标准库，不引第三方依赖；三个头文件之间只允许 `logger.h` / `thread_pool.h` 依赖 `blocking_queue.h`；
- 参考实现须在 ASan+UBSan 与 TSan 双构建下跑完三件套零报告；你的实现同样要过这一关（验收脚本里已包含）。

## 交付物与验收

在 `starter/` 的三个头文件里补齐 TODO，使：

```bash
bash tests/run_tests.sh   # 参考实现全过 + 双 sanitizer 干净；骨架（未改）必须不过
bash tests/run_tests.sh <你的构建目录>   # 只测你的实现：三项全过即达标
```

## 与毕业项目的关系

第六部分座舱信号网关的内部结构已经预定：CAN 解析线程 → `BlockingQueue` 分发 → `ThreadPool` 处理 → `Logger` 落盘，全是本项目的三个接口。所以这里的每个语义决策（关闭协议、过滤位置、时间戳取点）都不是练习题的偏好，是给毕业项目打的地基。

## 自测清单

- [ ] 能说清 `pop` 返回 `false` 的唯一条件，以及为什么"已关闭"还要让存量取完
- [ ] 有界队列被 `close()` 唤醒的生产者为什么要抛异常，而不是把元素塞进去
- [ ] `Logger::stop()` 返回后，凭什么断言"已入队的日志一条不少地在盘上"
- [ ] `log()` 与 `stop()` 并发时，为什么不会出现"关门瞬间入队、永远写不出"的那一条
- [ ] 为什么日志的时间戳要在前端取、格式却留在后端拼
- [ ] `shutdownNow()` 在 BlockingQueue 结构上是怎么实现"抛弃排队任务"的，代价是什么

## 验证记录（生产侧，测试先行）

- 验收脚本与三个验收程序先于参考实现写定；骨架首跑被判下（三项全红），失败形态与 TODO 一一对应：`queue_check` 卡在有界不阻塞；`logger_stress` 只落盘 213~1128 行（stop 不排空丢尾）且 DEBUG 未过滤；`pool_check` 排空计数为 0 或挂死超时（队列 pop 不阻塞，工作线程提前离场）。
- 参考实现：全套验收连跑三轮 6/6 全绿——`logger_stress` 落盘恰 8000 行（INFO=4000 / WARN=2000 / ERROR=2000，逐行校验通过）；ASan+UBSan、TSan 双构建零报告（TSan 在作者沙箱内以 `-no-pie` + `setarch -R` 运行，同第 5、6 章的说明）。
