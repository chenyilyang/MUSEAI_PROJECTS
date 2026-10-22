// ex01_fork_basic.cpp —— fork：一次调用、两次返回，从此代码分叉
// 编译基线：C++11
#include <cstdio>
#include <unistd.h>

int main() {
    std::printf("before fork: pid = %d\n", getpid());
    std::fflush(stdout);   // 关键动作：fork 会把 stdio 缓冲一并复制给子进程，
                           // 不先冲刷，这行在重定向到文件时可能出现两次（见正文）

    pid_t pid = fork();
    if (pid < 0) {
        std::perror("fork");
        return 1;
    }
    if (pid == 0) {
        // 子进程眼里 fork 返回 0
        std::printf("child : my pid = %d, my parent = %d\n", getpid(), getppid());
    } else {
        // 父进程眼里 fork 返回的是子进程的 PID
        std::printf("parent: my pid = %d, my child = %d\n", getpid(), pid);
    }
    // 父子各打印一行收尾，出现顺序由调度决定，不保证谁先
    std::printf("bye from %d\n", getpid());
    return 0;
}
