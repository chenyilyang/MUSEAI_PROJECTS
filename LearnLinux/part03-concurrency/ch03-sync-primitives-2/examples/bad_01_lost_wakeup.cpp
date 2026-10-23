// bad_01_lost_wakeup.cpp —— 反例：丢失唤醒（lost wakeup）
// 病灶有二，缺一不可地凑齐了这场事故：
//   1) 等待方用裸 wait——没有谓词状态，通知是一次性的"信号"，错过即不存在；
//   2) 通知方抢先 notify——worker 还在睡前准备时通知就已发出，落空。
// 运行预期：主线程打印通知后 join 永久挂住，timeout 杀掉时退出码 124。
// 正解在 ex01：状态（谓词）才是事实，通知只是提醒——提醒丢了没关系，
// 等待方醒来查状态；状态先立好，哪怕通知发在 wait 之前，谓词版 wait 也会直接通过。
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <thread>

static std::mutex g_m;
static std::condition_variable g_cv;

static void worker() {
    std::this_thread::sleep_for(std::chrono::milliseconds(300));  // 还在准备，没开始等
    std::printf("worker: about to wait (no predicate)\n");
    std::fflush(stdout);
    std::unique_lock<std::mutex> lk(g_m);
    g_cv.wait(lk);                              // 裸 wait：把命运交给"通知必须晚于我"
    std::printf("worker: woke up\n");           // 永不抵达
}

int main() {
    std::thread t(worker);
    g_cv.notify_one();                          // 此时根本没人 wait，通知落空
    std::printf("main: notified before anyone waited\n");
    std::fflush(stdout);
    t.join();                                   // 永久挂住
    std::printf("main: done\n");                // 永不抵达
    return 0;
}
