// ex03_posix_sem.cpp —— POSIX 计数信号量：限流的许可池
// 场景：6 个任务抢 3 个"许可"（如同时最多 3 路解码），拿不到许可的睡下等。
// 条件变量需要"状态+锁"才能拼出这个语义，信号量天生就是计数器。
// 注意：C++11 标准库没有信号量（std::counting_semaphore 是 C++20，见第 6 章），
// 本章用 POSIX sem_init/sem_wait/sem_post。
// 编译基线：C++11
#include <atomic>
#include <chrono>
#include <cstdio>
#include <semaphore.h>
#include <thread>
#include <vector>

static sem_t g_permits;
static std::atomic<int> g_inUse(0);
static std::atomic<int> g_maxUse(0);
static std::atomic<int> g_completed(0);

static void task() {
    sem_wait(&g_permits);                          // 拿许可；没有就在此睡下
    int cur = g_inUse.fetch_add(1) + 1;
    int prev = g_maxUse.load();                    // 顺手记水位（CAS 更新最大值）
    while (cur > prev && !g_maxUse.compare_exchange_weak(prev, cur)) {
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));   // 占着许可干活
    g_inUse.fetch_sub(1);
    ++g_completed;
    sem_post(&g_permits);                          // 归还许可，唤醒一个等待者
}

int main() {
    sem_init(&g_permits, 0, 3);                    // 0=进程内线程共享；初值 3 张许可
    std::vector<std::thread> pool;
    for (int i = 0; i < 6; ++i) pool.push_back(std::thread(task));
    for (size_t i = 0; i < pool.size(); ++i) pool[i].join();
    sem_destroy(&g_permits);
    std::printf("completed = %d\n", g_completed.load());
    std::printf("max concurrent = %d (limit 3)\n", g_maxUse.load());
    return 0;
}
