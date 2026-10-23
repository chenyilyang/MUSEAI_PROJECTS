# LearnLinux 第二部分逐章细化计划（Linux 开发起步）

> 本文件是 `2026-10-22-learnlinux-plan.md` 中 Task 8 的逐章补齐稿，粒度与第一部分一致（文件 / 知识点 / 前置 / 示例与核对要点）。章节标题与顺序以总计划路线图为准。执行约定不变：逐章确认制；每章示例先实测后写正文。

**本部分主线：** 先把工具握熟（编译、链接、调试），再进操作系统本体：文件与系统调用 → 进程的生老病死 → 守护进程 → 信号，最后用一个多进程日志收集工具收官。C++ 标准仍以 C++11 为基线；本部分的示例以 C++ 源码承载 POSIX/Linux API，编译选项保持 `-Wall -Wextra` 零警告。

### Task 8.1：第 1 章 工具链与环境搭建

**Files:** 目录 `LearnLinux/part02-linux-basics/ch01-toolchain/`；示例分四个子目录：`ex01_build_stages/hello.cpp`、`ex02_libraries/{calc.h, calc.cpp, main.cpp}`、`ex03_cmake_project/`（多文件 CMake 工程）、`ex04_gdb/{buggy_sum.cpp, fixed_sum.cpp}`；`examples/CMakeLists.txt` 构建 ex01 与 ex04（ex02、ex03 按正文命令手动构建，本身就是教学内容）。

**本章知识点：** 编译四阶段（预处理/编译/汇编/链接）与对应工具、g++ 常用选项（-std/-Wall/-Wextra/-g/-O、-I/-L/-l、-c）、符号与 nm、静态库（ar）与动态库（-fPIC/-shared）的制作与链接、ldd 与 LD_LIBRARY_PATH、CMake 工程组织（多文件、多目录）、GDB 基本功（断点/运行/单步/print 变量/watch 思路）、开发机环境对齐清单（版本核对命令）。
**前置：** 第一部分全部（尤其 DynArray 的构建方式）。
**示例与核对要点：**
- `ex01_build_stages`：对 hello.cpp 依次执行 -E/-S/-c 与链接，用 nm 对照目标文件（未定义符号 U）与可执行文件（T）的符号差异；核对：四步产物齐全，最终程序输出固定问候语。
- `ex02_libraries`：同一份 calc 分别做成 libcalc.a 与 libcalc.so，main 两种链接方式都跑通；核对：静态版 ldd 无 libcalc，动态版 ldd 出现 libcalc.so 且需 LD_LIBRARY_PATH 才能运行。
- `ex03_cmake_project`：多文件工程（main + greet + calc 三模块）用 CMake 构建；核对：输出与手工构建结果一致。
- `ex04_gdb`：buggy_sum 求 1..10 之和因初值错误得 56；正文给出 GDB 会话实录（断点、print sum/i 定位病根），fixed_sum 输出 55；核对：作者以 gdb 批处理实测会话关键输出后写入正文。

### Task 8.2：第 2 章 文件与系统调用

**Files:** 目录 `ch02-file-syscalls/`；示例 `ex01_open_write_read.cpp`、`ex02_errno.cpp`、`ex03_lseek.cpp`、`ex04_nonblock.cpp`。

**本章知识点：** 系统调用与库函数的区别、文件描述符（fd）与 0/1/2、open 的 flags 与 mode、read/write 的返回值语义（部分读写、返回 0 的含义）、close 与 fd 回收、errno 与 perror/strerror 的错误处理约定、lseek 与文件偏移（及对管道无效）、fcntl 设 O_NONBLOCK 与 EAGAIN 语义。
**前置：** 第 1 章（工具链）；第一部分指针与缓冲区基础。
**示例与核对要点：**
- `ex01`：open 写文件再读回校验；核对：读回内容与写入逐字一致，打印所用 fd（应为 3，说明 0/1/2 已被占用）。
- `ex02`（错误路径）：打开不存在文件，打印 errno 数值与 perror/strerror 文案三对照；核对：errno 为 ENOENT(2)。
- `ex03`：lseek 到指定偏移读文件尾部，及 SEEK_END 求文件大小；核对：读出内容与文件大小与正文给定的夹具文件一致。
- `ex04`：管道读端设非阻塞，空管道 read 立即返回 -1 且 errno=EAGAIN；写入数据后再读成功；核对：两段输出与正文一致，演示"非阻塞不等于失败"。

### Task 8.3：第 3 章 进程（一）

**Files:** 目录 `ch03-process-1/`；示例 `ex01_fork_basic.cpp`、`ex02_fork_exec.cpp`、`ex03_wait_status.cpp`、`ex04_zombie_orphan.cpp`。

**本章知识点：** 进程映像与 PID/PPID、fork 的一次调用两次返回与写时复制概念、子进程继承什么/不继承什么、exec 家族替换映像（与 fork 的分工）、exit/_exit 与缓冲区冲刷的坑、僵尸与孤儿的成因、wait/waitpid 回收与退出状态解码（WIFEXITED/WEXITSTATUS、信号终止）。
**前置：** 第 2 章（fd 的继承为伏笔）；第一部分全部。
**示例与核对要点：**
- `ex01`：fork 后父子各打印 PID/PPID 与分支标记；核对：三类行齐全，子进程 PPID 等于父进程 PID。
- `ex02`：子进程 execl 执行 /bin/echo 回显固定字符串，父进程等待；核对：子进程输出与父进程收尾行顺序（exec 后原代码不复存在这一事实以"exec 之后的那行打印永不出现"验证）。
- `ex03`：子进程以约定退出码（42）结束，父进程 waitpid 并解码打印；核对：打印退出码 42 且为正常退出。
- `ex04`：先制造僵尸（子退出、父 sleep 不回收，ps 状态为 Z），再演示孤儿（父先退出，子 PPID 变为 1 或最近 subreaper）；核对：以正文给定的 ps 命令观察到两种状态，正文说明回收义务在谁。

### Task 8.4：第 4 章 进程（二）与守护进程

**Files:** 目录 `ch04-process-2-daemon/`；示例 `ex01_session_group.cpp`、`ex02_daemon.cpp`。

**本章知识点：** 进程组与会话、控制终端、setsid 的作用与调用资格、守护进程标准写法（两次 fork、setsid、umask、chdir、重定向标准 fd、关闭继承 fd）、守护进程的日志去向（自管日志文件 vs syslog）、环境变量的读取与设置（getenv/setenv、environ 一瞥）、getpid 类自查手段。
**前置：** 第 3 章（fork/wait）；第 2 章（fd 重定向原理）。
**示例与核对要点：**
- `ex01`：打印进程的 PID/PGID/SID，子进程 setsid 后再打印对比；核对：新会话中 SID 等于子进程 PID、且不再有控制终端归属。
- `ex02`：完整守护进程：按标准步骤脱离终端，向固定路径日志文件按秒写心跳行（带计数）；核对：启动后查进程存在且 PPID 为 1（或 subreaper），日志文件行数随时间增长，用 SIGTERM 终止后日志出现收尾行。

### Task 8.5：第 5 章 信号

**Files:** 目录 `ch05-signals/`；示例 `ex01_signal_flag.cpp`、`ex02_sigaction.cpp`、`ex03_sigchld.cpp`、`ex04_alarm_timer.cpp`。

**本章知识点：** 信号的概念与常见信号清单、signal() 与 sigaction() 的差异与选型、处理函数的安全红线（异步信号安全：只能用可重入函数、只置 volatile sig_atomic_t 标志）、信号的阻塞与 sigprocmask 一瞥、SIGCHLD 驱动 waitpid 循环回收（WNOHANG、一次回收多个）、setitimer/SIGALRM 周期任务、kill 发送信号。
**前置：** 第 3、4 章（子进程与回收在此闭环）。
**示例与核对要点：**
- `ex01`：signal() 注册 SIGINT，处理函数只置标志，主循环检查标志打印收尾后退出；核对：以 kill -INT 触发，输出符合正文序列。
- `ex02`：同样场景改用 sigaction 并说明为何生产代码选它（语义明确、可屏蔽、可带 SA_RESTART）；核对：行为与 ex01 一致，正文对照两者差异清单。
- `ex03`：父进程 fork 三个子进程，SIGCHLD 处理中置标志，主循环 waitpid(WNOHANG) 循环回收并打印每个子进程退出码；核对：三个退出码全部回收、无僵尸残留。
- `ex04`：setitimer 每秒一记 SIGALRM，主循环按标志打印节拍计数，5 次后退出；核对：计数 1..5 依次输出（只断言序列不断言墙钟精度）。

### Task 8.6：第二部分阶段项目——多进程日志收集工具

**Files:** `LearnLinux/part02-linux-basics/project-log-collector/`：`README.md`（任务书）、`starter/`（骨架：主程序框架 + TODO）、`solutions/`（参考实现）。

**需求（任务书要点）：** 父进程 fork 出 N 个 worker 子进程；每个 worker 按固定节奏把带自身标记的日志行写入管道；父进程读管道、逐行加时间戳汇总进日志文件；父进程处理 SIGCHLD 无僵尸回收子进程，收到 SIGTERM 后通知子进程退出、回收齐全、写收尾行并退出。
**验收：** 参考实现运行固定时长后，日志文件行数与结构符合任务书（每 worker 行数确定值、时间戳单调、无缺行标记）；进程列表无僵尸；ASan/UBSan 构建无报告（信号处理函数只置标志，主循环做重活——此结构为验收条件之一，正文点明）。
**Interfaces:** 本项目的"管道收集 + 信号回收"骨架将在毕业项目的 CAN 接入进程管理中复用。

---

## 追记（2026-10-23）：自第 5 章起改用测试先行产出

奕良于 2026-10-23 确认的生产方式变更：第 1~4 章采用"示例先行、本机实测核对"；**自第 5 章起，每章先写验收测试脚本（`tests/run_tests.sh`）并跑出红灯，再写示例实现转绿**，测试脚本随章入库，章节验证记录须写明红→绿过程。第 5 章已按此执行（首跑构建阶段判红 passed=0；实现后 passed=5 且复跑稳定）。后续各部分沿用此方式，除非奕良另有指示。
