# 阶段项目：多进程日志收集器（Log Collector）（第二部分）

## 项目目标

把第二部分的知识一次总装：**fork**（第 3 章）造 worker、**管道**（第 2 章）汇数据、**信号**（第 5 章）管生死。你将写一个日志收集器：父进程 fork 出 N 个 worker 子进程，worker 把日志行经一根管道汇给父进程；父进程给每行盖上时间戳、统一写入日志文件，并负责把每个 worker 的"身后事"办干净——正常跑完无僵尸，被 SIGTERM 叫停时也能优雅收尾。

这正是车载中间件的日常形态：多个进程产生日志/事件、一个收集进程统一落盘，失控与泄漏都发生在"没人收尾"的地方。

## 程序规格

调用方式：

```bash
./log_collector <日志文件路径> <worker数 N> <每个worker行数 M>
```

### worker（子进程）

- 编号 i = 0..N-1；依次产出 M 行，内容固定为 `worker-<i> msg <seq>`，seq 从 1 到 M；
- **每行必须用一次 write 写完**（一行是不可分割的单位：管道里多写者并发时，小于 PIPE_BUF 的单次 write 是原子的，行与行不会互相穿插）；
- 行与行之间小睡约 2ms，模拟真实产出节奏；
- 写完关闭管道写端，以**退出码 i** 用 `_exit` 结束（第 3 章纪律：不冲刷继承来的缓冲）。

### 父进程

- fork 之前：建管道、用 sigaction 装好 SIGCHLD 与 SIGTERM 的处理函数——**处理函数只许置 `volatile sig_atomic_t` 标志，不许做任何其他事**（第 5 章红线，本项目的硬性验收条件）；
- fork 完所有 worker 后，**立刻关闭自己手里的管道写端副本**（不关，读端永远等不到 EOF）；
- 主循环：非阻塞地读管道（第 2 章的 O_NONBLOCK + EAGAIN），按行切分，给每完整行加上 `[YYYY-MM-DDTHH:MM:SS] ` 时间戳前缀，写入日志文件；
- SIGCHLD 标志置位时，用 `waitpid(-1, &status, WNOHANG)` **排干循环**回收已结束的 worker，登记各自退出码；
- 正常结束条件：管道 EOF（所有写端都关闭）**且** N 个 worker 全部回收——两个条件缺一不可。

### SIGTERM 优雅关闭

运行途中收到 SIGTERM 时：

1. 向所有尚存活的 worker 转发 SIGTERM；
2. 继续把管道里已有的存量数据读完、落盘（不许粗暴丢弃）；
3. 回收全部 worker；
4. 写日志收尾行，以退出码 0 结束。整个收尾应在数秒内完成。

### 输出契约（验收按此逐条核对）

- 日志文件每一行（含收尾行）带时间戳前缀：
  - 内容行：`[2026-10-23T09:20:31] worker-0 msg 1`
  - 收尾行（必须为最后一行）：`[...] collector shutdown: workers=N lines=L`，L 为内容行总数；
- 正常跑完时 L 必须恰为 N×M，且每个 worker 的 seq 1..M 各出现恰好一次；
- 标准输出在结束时打印三行：
  - `total lines = L`
  - `worker exit codes: 0 1 ... N-1`（按 worker 编号顺序）
  - `reaped N workers, no zombies (ECHILD)`（回收后以一次 waitpid 得到 ECHILD 自证无僵尸）

## 硬性约束

- C++11 基线，`-Wall -Wextra` 零警告；ASan+UBSan 构建下运行无任何报告；
- 信号处理函数里只有标志赋值——在 handler 里 waitpid、printf、malloc 都判违规；
- 父进程在收尾前不得调用阻塞式 wait/waitpid 干等（要边收日志边回收）；
- 不得用 sleep 轮询代替 SIGCHLD 驱动（轮询只能作为读管道节奏的一部分）。
- worker 继承的是父进程的整张信号处置表（第 3 章的继承规则）——它该如何对待父进程为自己装的那些 handler，想清楚再动手，验收 B 盯着这一点。

## 交付物与验收

- `starter/`：可编译的骨架（参数解析、管道与 worker 已备好；父进程目前只会闷头读到 EOF——不会回收、没有信号处理、没有收尾输出），其中的 TODO 注释就是你的工作清单；
- 验收脚本：`bash tests/run_tests.sh` 做全流程验收（构建并测参考实现 → 构建并测骨架，前者必须全过、后者必须不过 → 参考实现再以 ASan/UBSan 构建复跑一轮）；
- 测你自己的实现：把你的二进制路径作为参数传给它：`bash tests/run_tests.sh /你的/log_collector`。

> 本项目按测试先行方式产出：验收脚本 `tests/run_tests.sh` 先于骨架与参考实现写定；骨架经实测通不过验收（缺回收证明、SIGTERM 收尾不合格）。参考实现第一轮也曾被验收 B 判下：worker 经 fork 继承了父进程的 SIGTERM 处理函数，对转发来的关闭信号毫无反应、迟迟不退出——修正为"worker 启动先把信号处置恢复默认"后全项通过，ASan/UBSan 复跑干净（2026-10-23）。

## 自测清单

- [ ] 我能解释为什么父进程必须在 fork 后立刻关掉自己的管道写端副本
- [ ] 我的 worker 每行只 write 一次，我能说清这防的是什么
- [ ] 我的回收是 SIGCHLD 标志 + WNOHANG 排干循环，且能以 ECHILD 自证
- [ ] SIGTERM 后我的程序会转发信号给 worker、读完存量、写收尾行、以 0 退出
- [ ] 我的 handler 里除了置标志什么都没有
