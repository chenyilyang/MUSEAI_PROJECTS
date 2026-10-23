// 练习 4 参考答案：reapAll() 排干函数 + 8 个几乎同时结束的子进程
// 8 个孩子同一时刻附近结束，SIGCHLD 大概率合并——排干循环是对策。
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <sys/wait.h>
#include <unistd.h>

static volatile std::sig_atomic_t g_childEvent = 0;
static void onChld(int) { g_childEvent = 1; }

// 一次排干：把当前已结束的子进程全部回收，返回本次收走的个数。
static int reapAll() {
    int n = 0;
    for (;;) {
        int status = 0;
        pid_t p = waitpid(-1, &status, WNOHANG);
        if (p <= 0) break;
        std::printf("reaped child %d exit = %d\n", p,
                    WIFEXITED(status) ? WEXITSTATUS(status) : -1);
        ++n;
    }
    std::fflush(stdout);
    return n;
}

int main() {
    struct sigaction sa;
    sa.sa_handler = onChld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGCHLD, &sa, NULL) < 0) { std::perror("sigaction"); return 1; }

    const int kChildren = 8;
    for (int i = 0; i < kChildren; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            usleep(100 * 1000);       // 八个孩子约在同一时刻醒来退出
            _exit(21 + i);            // 退出码 21..28
        }
    }

    int total = 0;
    while (total < kChildren) {
        if (g_childEvent) {
            g_childEvent = 0;
            total += reapAll();
        }
        usleep(10000);
    }

    int status = 0;
    errno = 0;
    pid_t p = waitpid(-1, &status, WNOHANG);
    if (p == -1 && errno == ECHILD) {
        std::printf("reaped %d children, no zombies left (ECHILD)\n", total);
    } else {
        std::printf("unexpected leftover: waitpid=%d errno=%d\n", p, errno);
        return 1;
    }
    return 0;
}
