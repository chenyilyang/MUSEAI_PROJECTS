// ex01_atomic_counter.cpp —— 计数器第三次改写：不用锁，用 atomic
// 第 1 章普通 int（丢更新）→ 第 2 章 mutex（精确但要上锁）→ 本章 atomic。
// fetch_add 把"读-改-写"压成一条硬件原子操作，不丢、不睡、不进内核。
// 纯计数场景只用到原子性本身、不借它发布别的数据，所以用 relaxed 序即可——
// 内存序管的是"顺带捎带什么可见性"，与原子性是两回事（第 3 节展开）。
// 编译基线：C++11
#include <atomic>
#include <chrono>
#include <cstdio>
#include <mutex>
#include <thread>
#include <vector>

static const int kThreads = 4;
static const int kIters = 250000;

static long long nowMs() {
    return (long long)std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

static long runAtomic() {
    std::atomic<long> counter(0);
    std::vector<std::thread> pool;
    for (int t = 0; t < kThreads; ++t)
        pool.push_back(std::thread([&counter] {
            for (int i = 0; i < kIters; ++i)
                counter.fetch_add(1, std::memory_order_relaxed);
        }));
    for (size_t i = 0; i < pool.size(); ++i) pool[i].join();
    return counter.load();
}

static long runMutex() {
    long counter = 0;
    std::mutex m;
    std::vector<std::thread> pool;
    for (int t = 0; t < kThreads; ++t)
        pool.push_back(std::thread([&counter, &m] {
            for (int i = 0; i < kIters; ++i) {
                std::lock_guard<std::mutex> lk(m);
                ++counter;
            }
        }));
    for (size_t i = 0; i < pool.size(); ++i) pool[i].join();
    return counter;
}

int main() {
    long long t0 = nowMs();
    long a = runAtomic();
    long long t1 = nowMs();
    std::printf("atomic counter = %ld (expected %d), %lld ms\n", a, kThreads * kIters, t1 - t0);

    t0 = nowMs();
    long m = runMutex();
    t1 = nowMs();
    std::printf("mutex counter = %ld (expected %d), %lld ms\n", m, kThreads * kIters, t1 - t0);
    return 0;
}
