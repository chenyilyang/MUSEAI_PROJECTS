// ex02_spinlock.cpp —— 用 atomic_flag 手工拼一把自旋锁
// atomic_flag 是唯一保证无锁的原子类型，test_and_set 一步完成"查+占"。
// 自旋锁与 mutex 的区别不在正确性，在等待方式：mutex 抢不到就睡（让出 CPU），
// 自旋锁抢不到就原地打转。所以它只配用在临界区极短、且线程数不超核数的场合；
// 核数不够时它会把持锁者赖以运行的 CPU 也抢走，是经典的反优化。
// 本例仍套上 lock_guard 使用——它满足 Lockable 约定（lock/unlock 成员），RAII 纪律不变。
// 编译基线：C++11
#include <atomic>
#include <cstdio>
#include <mutex>   // 为 std::lock_guard
#include <thread>
#include <vector>

class SpinLock {
public:
    SpinLock() { flag_.clear(); }   // C++11 的 atomic_flag 必须显式置清（C++20 起默认清）
    void lock() {
        while (flag_.test_and_set(std::memory_order_acquire)) {
            // 原地自旋：什么都不做，等持锁者 clear
        }
    }
    void unlock() { flag_.clear(std::memory_order_release); }
private:
    std::atomic_flag flag_;
};

static const int kThreads = 4;
static const int kIters = 250000;
static long g_counter = 0;          // 普通变量，靠自旋锁保护
static SpinLock g_lock;

static void bump() {
    for (int i = 0; i < kIters; ++i) {
        std::lock_guard<SpinLock> lk(g_lock);
        ++g_counter;
    }
}

int main() {
    std::vector<std::thread> pool;
    for (int i = 0; i < kThreads; ++i) pool.push_back(std::thread(bump));
    for (size_t i = 0; i < pool.size(); ++i) pool[i].join();
    std::printf("spinlock counter = %ld (expected %d)\n", g_counter, kThreads * kIters);
    return 0;
}
