// ex02_raii_locks.cpp —— lock_guard 与 unique_lock：RAII 锁的两种身段
// 关键演示：异常穿过临界区时，锁由析构自动释放——手工 lock/unlock 在这条路径上必漏。
// 编译基线：C++11
#include <cstdio>
#include <mutex>
#include <stdexcept>

static long g_total = 0;
static std::mutex g_m;

static long snapshot() {
    std::lock_guard<std::mutex> lk(g_m);
    return g_total;
}

static void addWithGuard(long n) {
    std::lock_guard<std::mutex> lk(g_m);   // 简单、不可手动解锁、不可移动——够用就选它
    g_total += n;
}

static void addThenThrow(long n) {
    std::lock_guard<std::mutex> lk(g_m);
    g_total += n;
    throw std::runtime_error("boom");      // 异常穿过临界区：lk 析构，锁照样释放
}

int main() {
    addWithGuard(40);
    try {
        addThenThrow(2);
    } catch (const std::exception& e) {
        std::printf("caught: %s\n", e.what());
    }
    // 能在异常之后继续读到总数，本身就证明锁被释放了（否则这里永久挂住）
    std::printf("total after exception path = %ld\n", snapshot());   // 42

    // unique_lock 的独门身段：可以中途解锁——锁内只取快照，
    // 昂贵计算放到锁外做，最后再上锁提交。lock_guard 做不到这种两段式。
    long snap;
    {
        std::unique_lock<std::mutex> lk(g_m);
        snap = g_total;
        lk.unlock();                       // 显式提前放锁（unique_lock 才有）
    }
    long computed = snap * 2;              // 锁外计算，不占着茅坑
    {
        std::unique_lock<std::mutex> lk(g_m);
        g_total = computed;
    }
    std::printf("final total = %ld\n", snapshot());                  // 84
    return 0;
}
