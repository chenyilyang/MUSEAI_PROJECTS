// ex01_condvar_handoff.cpp —— 条件变量入门：一对一交接
// 谜题：主线程必须等 worker "就绪"后才发令，worker 必须等"发令"后才动作。
// 解法结构（背下来）：共享状态（谓词）+ mutex + 条件变量，三者缺一不可。
// 等待方永远用谓词版 wait；通知方先改状态、再 notify。
// 编译基线：C++11
#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <thread>

static std::mutex g_m;
static std::condition_variable g_cv;
static bool g_ready = false;   // 谓词状态：worker 就绪
static bool g_go = false;      // 谓词状态：主线程已发令

static void worker() {
    {
        std::lock_guard<std::mutex> lk(g_m);
        std::printf("worker: ready\n");
        std::fflush(stdout);
        g_ready = true;
    }
    g_cv.notify_one();                       // 状态先行，通知随后
    {
        std::unique_lock<std::mutex> lk(g_m);
        g_cv.wait(lk, [] { return g_go; });  // 谓词版 wait：醒来先查状态，没满足继续睡
    }
    std::printf("worker: go received\n");
    std::fflush(stdout);
}

int main() {
    std::thread t(worker);
    {
        std::unique_lock<std::mutex> lk(g_m);
        g_cv.wait(lk, [] { return g_ready; });
    }
    std::printf("main: worker is ready, giving go\n");
    std::fflush(stdout);
    {
        std::lock_guard<std::mutex> lk(g_m);
        g_go = true;
    }
    g_cv.notify_one();
    t.join();
    std::printf("main: done\n");
    return 0;
}
