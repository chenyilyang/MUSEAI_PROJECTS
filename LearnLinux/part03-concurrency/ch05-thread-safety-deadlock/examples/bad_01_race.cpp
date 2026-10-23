// bad_01_race.cpp —— 反例回访：第 1 章的共享计数原样
// 普通构建下它只是"结果不准"，不报错、不崩溃，看起来人畜无害；
// 用 -fsanitize=thread 构建运行，ThreadSanitizer 会把这对无同步的并发访问
// 精确点名：哪一行读、哪一行写、哪个变量、哪两个线程。
//   g++ -std=c++11 -g -fsanitize=thread -pthread bad_01_race.cpp
// 编译基线：C++11
#include <cstdio>
#include <thread>
#include <vector>

static const int kThreads = 4;
static const int kIters = 250000;
static long g_counter = 0;              // 无锁、无原子的共享计数——竞争本体

static void bump() {
    for (int i = 0; i < kIters; ++i) ++g_counter;
}

int main() {
    std::vector<std::thread> pool;
    for (int i = 0; i < kThreads; ++i) pool.push_back(std::thread(bump));
    for (size_t i = 0; i < pool.size(); ++i) pool[i].join();
    std::printf("shared counter = %ld (expected 1000000)\n", g_counter);
    return 0;
}
