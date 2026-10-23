# 第 5 章 练习（第二部分）

共 5 题。先自己动手，再到 `solutions/` 对照（文字题答案在 `solutions/answers.md`，代码题答案是可编译运行的源码；写完代码题后可以自己补断言脚本验收它们）。

## 练习 1（异步信号安全判断）

下面哪些操作可以出现在信号处理函数里？逐项给结论与一句话理由：

(a) 给一个 `volatile sig_atomic_t` 变量赋值为 1
(b) `printf("got signal %d\n", sig)` 打印一行日志
(c) 用 `write(2, msg, len)` 往标准错误写一小段预先备好的字节
(d) `malloc(64)` 分配一小块内存暂存数据
(e) 给一个普通 `int` 计数器做 `++`
(f) 调用自己写的 `logEvent()`，其内部用 std::string 拼接消息再写文件

## 练习 2（读代码：handler 里埋了三颗雷）

```cpp
static int g_reaped = 0;
static void onChld(int) {
    int status;
    pid_t p = waitpid(-1, &status, WNOHANG);       // 雷一的一部分
    if (p > 0) {
        printf("child %d done\n", p);              // 雷二
        ++g_reaped;                                // 雷三的一部分
    }
}
```

这段 handler 想"收到 SIGCHLD 就回收一个并计数"，共有三个层面的问题：信号的合并、所调用函数的安全性、共享变量的类型。逐个指出，并写出修正后的结构（不用完整代码，讲清 handler 与主循环各自该做什么）。

## 练习 3（写代码：优雅退出）

写程序：主循环每 200ms 打印一行 `working N`（N 递增）；用 sigaction 安装 SIGINT 处理函数，函数里只置停止标志；主循环发现标志后打印 `interrupted after N rounds, exit cleanly` 并以退出码 0 结束。自己用脚本验证：启动后约 0.7 秒发 SIGINT，程序应在半个节拍内收尾且退出码为 0。

## 练习 4（写代码：排干回收 8 个）

写程序：把"一次排干当前已结束子进程"封装成 `int reapAll()`；父进程用 sigaction 接管 SIGCHLD 后 fork 8 个子进程，每个睡 100ms 后以不同退出码（21..28）结束；父进程靠信号标志驱动 reapAll 直到 8 个全数回收，最后用 ECHILD 验证收尾并打印总数。

## 练习 5（思考题）

(a) 标准信号为什么会"丢"？如果有人想"用 SIGINT 到达次数计数"（比如按三次 Ctrl+C 才真退出），信号合并会不会让他数错？结合 ex01 的实现说清：什么情况下计数是可靠的、什么情况下不保证。
(b) SA_RESTART 到底替你省了什么事？不用它时，一个阻塞在 `read` 上的进程被信号打断会发生什么，代码要怎么写才不丢数据、不误判为错误？（往第 2 章的部分读写循环上靠。）
