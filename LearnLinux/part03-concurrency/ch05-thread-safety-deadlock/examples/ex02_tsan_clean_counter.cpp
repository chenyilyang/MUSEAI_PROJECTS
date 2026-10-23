// ex02_tsan_clean_counter.cpp —— 对照组：第 2 章的 mutex 计数
// 与 bad_01 同一份活，只多了把锁。用 -fsanitize=thread 构建运行：
// 零报告、退出码 0、结果精确。本章验收的判据就是这一"净"一"脏"的对照。
//   g++ -std=c++11 -g -fsanitize=thread -pthread ex02_tsan_clean_counter.cpp
// 编译基线：C++11
#include <cstdio>
#include <mutex>
#include <thread>
#include <vector>

static const int kThreads = 4;
static const int kIters = 250000;
static long g_counter = 0;
static std::mutex g_m;

static void bump() {
    for (int i = 0; i < kIters; ++i) {
        std::lock_guard<std::mutex> lk(g_m);
        ++g_counter;
    }
}

int main() {
    std::vector<std::thread> pool;
    for (int i = 0; i < kThreads; ++i) pool.push_back(std::thread(bump));
    for (size_t i = 0; i < pool.size(); ++i) pool[i].join();
    std::printf("mutex counter = %ld (expected 1000000)\n", g_counter);
    return 0;
}
