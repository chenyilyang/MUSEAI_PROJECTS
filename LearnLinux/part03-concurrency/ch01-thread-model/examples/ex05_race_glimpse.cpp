// ex05_race_glimpse.cpp —— 竞争初瞥：为什么下一章要学锁
// 同一份活两种干法：
//   A) 4 个线程共同累加一个普通 int —— 读-改-写不是原子的，更新会丢失；
//   B) 4 个线程各自累加自己的计数 —— 互不干扰，分毫不差。
// 本章先看现象、立账：A 的确切数值每次运行都不同（只断言 ≤ 应得数），
// 定位与修复在第 2 章（mutex）与第 5 章（TSan 实测）。
// 编译基线：C++11
#include <cstdio>
#include <thread>
#include <vector>

static const int kThreads = 4;
static const int kIters = 250000;

static long g_shared = 0;                    // 故意不设防的共享计数
static void bumpShared() {
    for (int i = 0; i < kIters; ++i) ++g_shared;
}

static long g_own[kThreads] = {0, 0, 0, 0};  // 每人一格
static void bumpOwn(int idx) {
    for (int i = 0; i < kIters; ++i) ++g_own[idx];
}

int main() {
    {
        std::vector<std::thread> pool;
        for (int i = 0; i < kThreads; ++i) pool.push_back(std::thread(bumpShared));
        for (size_t i = 0; i < pool.size(); ++i) pool[i].join();
    }
    std::printf("shared counter = %ld (expected %d)\n", g_shared, kThreads * kIters);

    {
        std::vector<std::thread> pool;
        for (int i = 0; i < kThreads; ++i) pool.push_back(std::thread(bumpOwn, i));
        for (size_t i = 0; i < pool.size(); ++i) pool[i].join();
    }
    long total = 0;
    for (int i = 0; i < kThreads; ++i) total += g_own[i];
    std::printf("separate total = %ld (expected %d)\n", total, kThreads * kIters);
    return 0;
}
