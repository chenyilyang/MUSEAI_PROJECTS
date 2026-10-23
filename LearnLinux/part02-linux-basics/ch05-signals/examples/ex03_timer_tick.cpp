// ex03_timer_tick.cpp —— 定时器信号：setitimer + SIGALRM 做周期节拍
// 处理函数依然只做一件事：给 sig_atomic_t 计数器 +1。
// 摘要打印与收尾都在主循环里，节奏：每 500ms 一拍，2、4 拍打印摘要，满 6 拍停表收尾。
// 编译基线：C++11
#include <csignal>
#include <cstdio>
#include <sys/time.h>
#include <unistd.h>

static volatile std::sig_atomic_t g_ticks = 0;
static void onAlrm(int) { ++g_ticks; }

int main() {
    struct sigaction sa;
    sa.sa_handler = onAlrm;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGALRM, &sa, NULL) < 0) { std::perror("sigaction"); return 1; }

    struct itimerval it;
    it.it_value.tv_sec = 0;
    it.it_value.tv_usec = 500 * 1000;       // 首拍 500ms 后
    it.it_interval = it.it_value;           // 之后每 500ms 一拍
    if (setitimer(ITIMER_REAL, &it, NULL) < 0) { std::perror("setitimer"); return 1; }

    int reported = 0;
    for (;;) {
        int t = (int)g_ticks;
        if (t >= 2 && reported < 2) {
            std::printf("tick summary: ticks = 2\n");
            std::fflush(stdout);
            reported = 2;
        }
        if (t >= 4 && reported < 4) {
            std::printf("tick summary: ticks = 4\n");
            std::fflush(stdout);
            reported = 4;
        }
        if (t >= 6) {
            struct itimerval off;
            off.it_value.tv_sec = 0; off.it_value.tv_usec = 0;
            off.it_interval = off.it_value;
            setitimer(ITIMER_REAL, &off, NULL);    // 先停表，再报总数
            std::printf("total ticks = %d\n", t);
            break;
        }
        usleep(10000);
    }
    return 0;
}
