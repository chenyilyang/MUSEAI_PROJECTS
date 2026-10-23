// 练习 3 参考答案：优雅退出——SIGINT 只置标志，主循环收尾
#include <csignal>
#include <cstdio>
#include <unistd.h>

static volatile std::sig_atomic_t g_stop = 0;
static void onInt(int) { g_stop = 1; }   // 处理函数只做这一件事

int main() {
    struct sigaction sa;
    sa.sa_handler = onInt;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) < 0) { std::perror("sigaction"); return 1; }

    int rounds = 0;
    while (!g_stop) {
        ++rounds;
        std::printf("working %d\n", rounds);
        std::fflush(stdout);
        usleep(200 * 1000);   // 被 SIGINT 打断会提前醒，循环条件随即生效
    }
    std::printf("interrupted after %d rounds, exit cleanly\n", rounds);
    return 0;
}
