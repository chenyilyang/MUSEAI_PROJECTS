// ex03_environ.cpp —— 环境变量：进程的"启动配置"，且会遗传
// 编译基线：C++11
#include <cstdio>
#include <cstdlib>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    const char* home = std::getenv("HOME");
    std::printf("HOME = %s\n", home != NULL ? home : "(unset)");

    // 自己设一个：1 表示允许覆盖已有值
    setenv("LEARNLINUX_DEMO", "cockpit", 1);
    std::printf("parent sees LEARNLINUX_DEMO = %s\n", std::getenv("LEARNLINUX_DEMO"));
    std::fflush(stdout);   // fork 前冲刷（第 3 章的纪律）

    pid_t pid = fork();
    if (pid == 0) {
        // 子进程继承 fork 时刻的整张环境表
        std::printf("child inherits LEARNLINUX_DEMO = %s\n", std::getenv("LEARNLINUX_DEMO"));
        std::fflush(stdout);   // _exit 不冲刷缓冲，手动冲
        // 子进程改自己的，父进程那份不受影响（环境也是写时复制语义）
        setenv("LEARNLINUX_DEMO", "child-only", 1);
        _exit(0);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    std::printf("parent still sees LEARNLINUX_DEMO = %s\n", std::getenv("LEARNLINUX_DEMO"));
    return 0;
}
