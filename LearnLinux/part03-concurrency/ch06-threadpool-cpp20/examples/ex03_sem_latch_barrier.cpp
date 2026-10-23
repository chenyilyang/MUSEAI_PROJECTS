// ex03：C++20 专题（二）—— counting_semaphore + latch + barrier【-std=c++20 构建】
// 三件新原语，各管一段：
//   latch              一次性的"发令枪"：计数减到 0 后所有 wait 放行，且不可重置；
//                      这里 6 个工作线程先齐步等候，主线程一声令下同时出发，
//                      保证后面的限流实测是从同一条起跑线开始的。
//   counting_semaphore 计数信号量的标准版（第 3 章 POSIX sem 的同门）：
//                      初值 3 —— 任一时刻最多 3 个任务在临界区里，实测最大值断言。
//   barrier            可复用的分阶段栅栏：所有线程到齐才一起进入下一阶段，
//                      完成回调在放行之前执行——拿它打印分界线，日志严格分段。
#include <atomic>
#include <barrier>
#include <chrono>
#include <cstdio>
#include <latch>
#include <mutex>
#include <semaphore>
#include <thread>
#include <vector>

namespace {
const int kWorkers = 6;
const int kPhases = 2;
const int kLimit = 3;

std::latch go(1);                                   // 发令枪：主线程 count_down 放行
std::counting_semaphore<kWorkers> sem(kLimit);      // 限流 3
std::barrier phaseDone(kWorkers, [] {               // 完成回调类型由构造实参推导（CTAD）
    std::printf("--- phase boundary: all workers arrived ---\n");
    std::fflush(stdout);
});
std::atomic<int> inFlight(0);
std::atomic<int> maxInFlight(0);
std::mutex printM;                                  // 只保护整行输出不交错

void worker(int id) {
    go.wait();                                      // 齐步等候
    for (int phase = 0; phase < kPhases; ++phase) {
        sem.acquire();
        int cur = inFlight.fetch_add(1) + 1;
        int seen = maxInFlight.load();
        while (cur > seen && !maxInFlight.compare_exchange_weak(seen, cur)) {
        }
        {
            std::lock_guard<std::mutex> lk(printM);
            std::printf("worker %d: phase %d work\n", id, phase);
            std::fflush(stdout);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        inFlight.fetch_sub(1);
        sem.release();
        phaseDone.arrive_and_wait();                // 本阶段全员到齐才一起往下走
    }
}
}  // namespace

int main() {
    std::vector<std::thread> ts;
    for (int i = 0; i < kWorkers; ++i) ts.push_back(std::thread(worker, i));
    go.count_down();                                // 发令：6 个线程同时出发
    for (std::size_t i = 0; i < ts.size(); ++i) ts[i].join();
    std::printf("max concurrent = %d (limit %d)\n", maxInFlight.load(), kLimit);
    return maxInFlight.load() == kLimit ? 0 : 1;
}
