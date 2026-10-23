// ex02_sigchld_reap.cpp —— SIGCHLD 驱动回收：兑现第 3 章练习 5 的答案
// 要点：
//  1) 子进程结束时内核给父进程发 SIGCHLD，处理函数只置一个标志；
//  2) 标准信号不排队：5 个子进程接连结束，信号可能合并成 1 个——
//     所以回收必须是 waitpid(-1, WNOHANG) 的"排干"循环，只收一个必留僵尸；
//  3) 全部收完后，再 waitpid 应得 -1/ECHILD，这是"无僵尸"的程序化证明。
// 编译基线：C++11
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <sys/wait.h>
#include <unistd.h>

static volatile std::sig_atomic_t g_childEvent = 0;
static void onChld(int) { g_childEvent = 1; }   // 只置标志，回收在主循环里做

int main() {
    struct sigaction sa;
    sa.sa_handler = onChld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGCHLD, &sa, NULL) < 0) { std::perror("sigaction"); return 1; }

    const int kChildren = 5;
    for (int i = 0; i < kChildren; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            usleep((useconds_t)(i + 1) * 100 * 1000);   // 寿命 100..500ms，错峰结束
            _exit(11 + i);                              // 退出码 11..15，各不相同
        }
    }

    int reaped = 0;
    while (reaped < kChildren) {
        if (g_childEvent) {
            g_childEvent = 0;
            // 排干：此刻可能有多个子进程已结束（信号合并过），必须循环收到"暂时没有"为止
            for (;;) {
                int status = 0;
                pid_t p = waitpid(-1, &status, WNOHANG);
                if (p <= 0) break;
                std::printf("reaped child %d exit = %d\n", p,
                            WIFEXITED(status) ? WEXITSTATUS(status) : -1);
                std::fflush(stdout);
                ++reaped;
            }
        }
        usleep(10000);
    }

    // 收尾验证：已经没有任何子进程（活着的或僵尸的）了
    int status = 0;
    errno = 0;
    pid_t p = waitpid(-1, &status, WNOHANG);
    if (p == -1 && errno == ECHILD) {
        std::printf("no zombies left (ECHILD)\n");
    } else {
        std::printf("unexpected: waitpid returned %d errno = %d\n", p, errno);
        return 1;
    }
    return 0;
}
