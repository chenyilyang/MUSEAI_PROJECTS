// ex01_session_group.cpp —— 进程组与会话：setsid 前后对比
// 概念：若干进程成一组（PGID），若干进程组开一次"会话"（SID），
// 会话可以绑定一个控制终端。setsid 让调用者自立门户开新会话——
// 但有个资格限制：进程组组长不能调用 setsid，所以要先 fork。
// 编译基线：C++11
#include <cstdio>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    std::printf("launcher: pid = %d, pgid = %d, sid = %d\n",
                getpid(), getpgrp(), getsid(0));
    std::fflush(stdout);   // 第 3 章的纪律：fork 前先冲刷，免得缓冲被子进程继承后重复打印

    pid_t pid = fork();
    if (pid < 0) { std::perror("fork"); return 1; }
    if (pid == 0) {
        std::printf("child before setsid: pid = %d, pgid = %d, sid = %d\n",
                    getpid(), getpgrp(), getsid(0));
        std::fflush(stdout);   // 子进程用 _exit 收尾，不冲刷缓冲——打印后手动冲
        // 此刻子进程不是组长（它在父进程的组里），有资格开新会话
        if (setsid() < 0) { std::perror("setsid"); _exit(1); }
        std::printf("child after  setsid: pid = %d, pgid = %d, sid = %d\n",
                    getpid(), getpgrp(), getsid(0));
        std::fflush(stdout);
        // 三个数应当相等：它既是新会话的老大，也是新进程组的组长，
        // 并且不再挂在任何控制终端名下。
        _exit(0);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    return 0;
}
