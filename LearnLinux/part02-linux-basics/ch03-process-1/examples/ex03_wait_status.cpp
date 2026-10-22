// ex03_wait_status.cpp —— 回收子进程，并把退出状态解码出来
// status 是一个位段，必须用 WIFEXITED/WEXITSTATUS/WIFSIGNALED/WTERMSIG 宏解码，
// 不能直接当整数读。
// 编译基线：C++11
#include <cstdio>
#include <csignal>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    // —— 第一幕：正常退出，退出码 42 ——
    pid_t p1 = fork();
    if (p1 == 0) {
        _exit(42);                     // 子进程带着约定退出码离开
    }
    int status = 0;
    waitpid(p1, &status, 0);
    std::printf("child %d: exited = %d, code = %d\n",
                p1, WIFEXITED(status) ? 1 : 0,
                WIFEXITED(status) ? WEXITSTATUS(status) : -1);   // exited=1, code=42

    // —— 第二幕：被信号终止 ——
    pid_t p2 = fork();
    if (p2 == 0) {
        raise(SIGTERM);                // 自己给自己发终止信号
        _exit(0);                      // 走不到这里
    }
    waitpid(p2, &status, 0);
    std::printf("child %d: signaled = %d, sig = %d\n",
                p2, WIFSIGNALED(status) ? 1 : 0,
                WIFSIGNALED(status) ? WTERMSIG(status) : -1);    // signaled=1, sig=15
    return 0;
}
