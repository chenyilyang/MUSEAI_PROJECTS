// ex01_signal_vs_sigaction.cpp —— 同一场景，两种装法：signal() 与 sigaction()
// 场景：进程等 SIGINT，处理函数只做一件事——给 sig_atomic_t 计数器 +1；
//       主循环数满 2 次后打印收尾。挨第一下不死，是"默认处置被替换"的实证。
// 用法：./ex01_signal_vs_sigaction signal    （老式简装）
//       ./ex01_signal_vs_sigaction sigaction （正装：可控掩码与标志）
// 编译基线：C++11
#include <csignal>
#include <cstdio>
#include <cstring>
#include <unistd.h>

// 处理函数能碰的东西极少：对 volatile sig_atomic_t 的读写是安全的经典操作。
// printf、malloc、任何带锁的东西都不许出现在这里（见正文"异步信号安全"）。
static volatile std::sig_atomic_t g_ints = 0;
static void onInt(int) { ++g_ints; }

int main(int argc, char* argv[]) {
    const char* mode = (argc > 1) ? argv[1] : "sigaction";

    if (std::strcmp(mode, "signal") == 0) {
        // 简装：一行搞定，但语义含糊——处理期间是否屏蔽同种信号、
        // 被打断的系统调用是否自动重启，历史上各平台不一，不可控。
        signal(SIGINT, onInt);
    } else {
        // 正装：sigaction 结构体把三件事都摆在明面上——
        // 处理函数、处理期间额外屏蔽的信号集（sa_mask）、行为标志（sa_flags）。
        struct sigaction sa;
        sa.sa_handler = onInt;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_RESTART;   // 被信号打断的慢系统调用自动重启，不用自己重试
        if (sigaction(SIGINT, &sa, NULL) < 0) { std::perror("sigaction"); return 1; }
    }

    std::printf("mode = %s: waiting for 2 SIGINTs (Ctrl+C or kill -INT)\n", mode);
    std::fflush(stdout);
    while (g_ints < 2) {
        usleep(20000);              // 主循环只管轮询计数，重活永远在这里干
    }
    std::printf("mode = %s, interrupts = %d, bye\n", mode, (int)g_ints);
    return 0;
}
