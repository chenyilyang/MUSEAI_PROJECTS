# 第 4 章 进程（二）与守护进程（第二部分 · Linux 开发起步）

## 本章目标

学完这一章，你应该能做到：

- 说清进程组、会话、控制终端三者的关系，解释 setsid 的作用与调用资格；
- 按标准步骤亲手写出一个守护进程：两次 fork、setsid、umask、chdir、标准流重定向，一步不缺；
- 说清守护进程的日志该往哪儿去、为什么不能用 printf；
- 会用 getenv/setenv 读写环境变量，知道它随 fork 继承、且改动不回流父进程。

**前置知识**：第二部分第 3 章（fork/exec/wait、fork 前 fflush 与子进程用 _exit 的纪律——本章示例里继续在用）。本章的 SIGTERM 处理只用最简形式，信号的完整机制在下一章专讲。

**本章文件**：示例在 `examples/`（3 个），练习在 `exercises.md`，参考答案在 `solutions/`。

---

## 1. 进程组、会话与控制终端

进程之上还有两层组织：

- **进程组（PGID）**：一组相关进程。shell 用它管理前后台作业——你在终端按 Ctrl+C，信号是发给整个前台进程组的。
- **会话（SID）**：若干进程组的集合，通常对应一次登录。会话可以绑定一个**控制终端**；会话首进程打开的终端设备就可能成为它的控制终端，这也是"关掉终端，进程收到 SIGHUP 连带死亡"的来历。

查三者的工具是 `getpid() / getpgrp() / getsid(0)`。`ex01_session_group.cpp` 让子进程在 setsid 前后各报一次家门，实测（只看关系，数值每次都不同）：

```
launcher:            pid = 5500, pgid = 5452, sid = 0*
child before setsid: pid = 5501, pgid = 5452, sid = 0*   ← 还在启动者的组与会话里
child after  setsid: pid = 5501, pgid = 5501, sid = 5501  ← 三个值归于自身：自立门户
```

`setsid()` 让调用者成为**新会话的首进程兼新进程组的组长**，并与原控制终端脱钩。它的资格限制是理解守护进程写法的钥匙：**进程组组长不能调用 setsid**（组长自立门户会让组员归属混乱）。刚 fork 出的子进程身处父进程的组里、不是组长，所以"先 fork、再 setsid"不是仪式感，是资格使然。

（*作者的验证沙箱没有常规的登录会话，启动者一行的 sid 显示为 0，这是该环境的特殊形态；在你自己的终端里，这里会是登录 shell 所在会话的 SID。要看的只有关系：setsid 前子进程与启动者同组同会话，setsid 后它三值归一。）

## 2. 守护进程的标准写法

守护进程（daemon）就是脱离终端、常驻后台的服务进程。`ex02_daemon.cpp` 是完整版，步骤与理由逐一对应：

| 步骤 | 做什么 | 为什么 |
|---|---|---|
| 1 | fork，父进程退出 | 让启动命令立刻返回；子进程被过继，且不是组长、取得 setsid 资格 |
| 2 | setsid | 开新会话，脱离原控制终端——原终端关闭也不会再牵连它 |
| 3 | 再 fork 一次 | 新进程不是会话首进程，今后 open 任何终端设备都不会被认作控制终端，与终端彻底绝缘 |
| 4 | umask(0) | 文件权限不再受启动者 umask 的意外影响，由自己的 mode 说了算 |
| 5 | chdir("/") | 不钉住启动时所在的目录——否则那个目录（比如某个挂载点）永远无法卸载 |
| 6 | 0/1/2 全部改指 /dev/null | 读不到终端输入，也不会有零散输出打到任何终端上；且占住 0/1/2 号位，免得后续 open 的文件被当成标准流误用（第 2 章 fd 分配规则的应用题） |
| 7 | 写 pid 文件，此后只写日志文件 | 没有终端了，日志要有别的去处；pid 文件是外界找到并管理它的把手 |

主循环每秒往 `/tmp/learnlinux_daemon.log` 追加一行心跳。在作者环境实测的完整生命周期：

```
启动后查 ps：
  PID  PPID  PGID  SID  TT   CMD
  5313   1    5312  5312  ?   ./ex02_daemon      ← PPID 已是 1；TTY 为 ?（没有控制终端）

日志随时间增长：
  heartbeat 1 from daemon pid 5313
  heartbeat 2 from daemon pid 5313
  ...

kill -TERM 5313 之后，日志多出收尾一行，进程消失：
  daemon exiting on SIGTERM
```

注意 PGID/SID 是 5312 而守护进程本体是 5313——5312 是第二次 fork 前那个中间进程，它完成"让最终进程不是会话首进程"的使命后就退出了，这正是步骤 3 的实证。SIGTERM 的处理函数只做一件事：置一个 `sig_atomic_t` 标志，主循环每秒醒来时检查它再收尾——为什么处理函数里只能做这么点事，是下一章的正题，这里先记住这条纪律。

**日志去向**补两句：本章用"自管日志文件"（简单、自包含，轮转与清理要自己管）；系统级服务更常见的是打进 **syslog/journald**（由系统统一收集、轮转、按级别过滤，代价是格式与排查方式要按系统的规矩来）。车载中间件进程两种都会见到，先把文件版写熟。

## 3. 环境变量：进程的启动配置

每个进程带一张**环境变量表**（键=值字符串），由父进程在 fork/exec 时传给子进程——它是进程"启动配置"的最古老载体。`ex03_environ.cpp` 实测三件事：

```
HOME = /home/hatch
parent sees LEARNLINUX_DEMO = cockpit
child inherits LEARNLINUX_DEMO = cockpit        ← 子进程继承 fork 时刻的表
parent still sees LEARNLINUX_DEMO = cockpit     ← 子进程改了自己的，父进程纹丝不动
```

规则：`getenv` 读、`setenv(key, value, overwrite)` 写；**继承是单向快照**——子进程拿到的是 fork 那一刻的副本，之后各改各的，永不回流。这正是"在 shell 里 `export` 之后启动程序才生效、在程序里 setenv 影响不了已运行的父进程"的原理。练习 4 会让你用它做一次真实的配置读取（用环境变量决定日志目录）。

---

## 示例代码：构建与运行

```bash
cd examples
cmake -S . -B build
cmake --build build
./build/ex01_session_group
./build/ex03_environ
./build/ex02_daemon
# 守护进程观察三件套：
cat /tmp/learnlinux_daemon.pid
ps -o pid,ppid,pgid,sid,tty,cmd -p $(cat /tmp/learnlinux_daemon.pid)
tail -f /tmp/learnlinux_daemon.log
kill -TERM $(cat /tmp/learnlinux_daemon.pid)
```

核对要点：

- `ex01`：setsid 前子进程与启动者同 pgid/sid；setsid 后子进程 pid、pgid、sid 三值相等。
- `ex02`：启动器立即返回；守护进程 PPID 为 1、TTY 为 `?`、PGID/SID 是中间进程留下的会话号（不等于守护进程自身 PID）；日志每秒长一行；SIGTERM 后出现收尾行且进程消失。
- `ex03`：四行输出齐全，子进程继承到 cockpit，父进程在子进程改值后仍为 cockpit。

> 验证记录：以上结果由作者在 Ubuntu 24.04 / GCC 13.3 环境下以 `-std=c++11 -Wall -Wextra` 构建实测（零警告），守护进程的进程状态经 ps 实测，日期 2026-10-23。启动者 sid 显示为 0 是作者沙箱会话的特殊形态，已在正文标注。

## 动手练习

见本章 `exercises.md`（5 道题；参考答案在 `solutions/`）。

## 自测清单

- [ ] 我能画出进程、进程组、会话、控制终端的从属关系，并解释 setsid 的资格限制
- [ ] 我能不看稿写出守护进程的七个步骤，并逐一说出理由
- [ ] 我知道守护进程为什么不能用 printf、日志有哪两个正经去向、pid 文件的作用
- [ ] 我解释得了"第二次 fork"防止的是什么（重新获得控制终端）
- [ ] 我会用 getenv/setenv，并能预测子进程对环境变量的修改会不会影响父进程

## 本章小结与下一章预告

进程部分收尾：你会生（fork）、会变身（exec）、会收尾（wait）、会编组与脱钩（session/setsid），还能把进程做成常驻服务（daemon）。**下一章（信号）**补上最后一块拼图：信号的完整机制——sigaction 的正确姿势、处理函数的安全红线（异步信号安全）、用 SIGCHLD 系统性回收子进程（替第 3 章练习 5 埋的问题兑现答案）、以及用定时器信号做周期任务。学完它，第二部分就只剩阶段项目了。
