// ex04_lock_cost.cpp —— 锁的开销实测：同一份活，三种策略
//   racy        不加锁（结果错误，图快）
//   coarse lock 每线程只上一次锁、罩住整个循环（串行化，但上锁次数最少）
//   fine lock   每次累加都上锁（粒度最细，上锁 100 万次 + 高频争抢）
// 断言只管正确性；耗时随机器而变，正文引用的是作者环境实测值，读时看量级与成因。
// 编译基线：C++11
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

static long runRacy() {
    long counter = 0;
    std::vector<std::thread> pool;
    for (int t = 0; t < kThreads; ++t)
        pool.push_back(std::thread([&counter] {
            for (int i = 0; i < kIters; ++i) ++counter;
        }));
    for (size_t i = 0; i < pool.size(); ++i) pool[i].join();
    return counter;
}

static long runCoarse() {
    long counter = 0;
    std::mutex m;
    std::vector<std::thread> pool;
    for (int t = 0; t < kThreads; ++t)
        pool.push_back(std::thread([&counter, &m] {
            std::lock_guard<std::mutex> lk(m);       // 一把锁罩全程：其余线程干等
            for (int i = 0; i < kIters; ++i) ++counter;
        }));
    for (size_t i = 0; i < pool.size(); ++i) pool[i].join();
    return counter;
}

static long runFine() {
    long counter = 0;
    std::mutex m;
    std::vector<std::thread> pool;
    for (int t = 0; t < kThreads; ++t)
        pool.push_back(std::thread([&counter, &m] {
            for (int i = 0; i < kIters; ++i) {
                std::lock_guard<std::mutex> lk(m);   // 每次累加一次上锁/解锁
                ++counter;
            }
        }));
    for (size_t i = 0; i < pool.size(); ++i) pool[i].join();
    return counter;
}

int main() {
    long long t0 = nowMs();
    long r = runRacy();
    long long t1 = nowMs();
    // 无锁版的结果不可复现：多数时候短缺，偶尔也会"恰好全对"——后者更危险，
    // 它会让你误以为代码没问题。所以这里的判语按实际结果动态生成：
    std::printf("racy        : shared = %ld (%s), %lld ms\n", r,
                r == kThreads * kIters ? "exact -- lucky this time" : "wrong", t1 - t0);

    t0 = nowMs();
    long c = runCoarse();
    t1 = nowMs();
    std::printf("coarse lock : shared = %ld (exact), %lld ms\n", c, t1 - t0);

    t0 = nowMs();
    long f = runFine();
    t1 = nowMs();
    std::printf("fine lock   : shared = %ld (exact), %lld ms\n", f, t1 - t0);
    return 0;
}
