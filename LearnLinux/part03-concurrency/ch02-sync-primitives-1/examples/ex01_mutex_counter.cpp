// ex01_mutex_counter.cpp —— 还第 1 章的账：共享计数 + mutex = 精确
// 与 ex05_race_glimpse 同一份活，唯一区别是每次累加都在锁内。
// 锁把"读-改-写"三步圈成不可分割的临界区，丢失更新就此消失。
// 编译基线：C++11
#include <cstdio>
#include <mutex>
#include <thread>
#include <vector>

static const int kThreads = 4;
static const int kIters = 250000;

static long g_counter = 0;
static std::mutex g_m;                 // 与计数绑定一生的那把锁（同一数据同一锁，见正文）

static void bump() {
    for (int i = 0; i < kIters; ++i) {
        std::lock_guard<std::mutex> lk(g_m);   // 上锁；出作用域自动解锁（RAII）
        ++g_counter;
    }                                          // 每轮迭代末尾锁释放——粒度最细的写法
}

int main() {
    std::vector<std::thread> pool;
    for (int i = 0; i < kThreads; ++i) pool.push_back(std::thread(bump));
    for (size_t i = 0; i < pool.size(); ++i) pool[i].join();
    std::printf("mutex counter = %ld (expected %d)\n", g_counter, kThreads * kIters);
    return 0;
}
