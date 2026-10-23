// ex01_deadlock_fixed.cpp —— bad_02 的修复版：双向转账
// 场景升级为真实业务：两个账户各有自己的锁，线程 1 反复 A→B 转账，
// 线程 2 反复 B→A——天生就是反序加锁的地形。两种标准解法：
//   1) 全局锁顺序：所有人一律先锁地址小的那把（纪律解法）；
//   2) std::lock：一次把多把锁"要么全拿到、要么不拿"地锁住（算法解法，本例用它）。
// std::lock 之后再用 lock_guard 以 adopt_lock 接管所有权，RAII 纪律不丢。
// 两边各转 1000 次、每次 1 元：终局两账户必须各回 1000，总额 2000 守恒。
// 编译基线：C++11
#include <cstdio>
#include <mutex>
#include <thread>

struct Account {
    Account(long b) : balance(b) {}
    std::mutex m;          // mutex 不可拷贝/移动：账户总是原地构造
    long balance;
};

static Account g_a(1000);
static Account g_b(1000);

static void transfer(Account& from, Account& to, long amount) {
    std::lock(from.m, to.m);                                // 同时锁两把，不留环
    std::lock_guard<std::mutex> lk1(from.m, std::adopt_lock);
    std::lock_guard<std::mutex> lk2(to.m, std::adopt_lock);
    from.balance -= amount;
    to.balance += amount;
}

static void moveAB() {
    for (int i = 0; i < 1000; ++i) transfer(g_a, g_b, 1);
}

static void moveBA() {
    for (int i = 0; i < 1000; ++i) transfer(g_b, g_a, 1);
}

int main() {
    std::thread t1(moveAB);
    std::thread t2(moveBA);
    t1.join();
    t2.join();
    std::printf("A = %ld, B = %ld, total = %ld (expected)\n",
                g_a.balance, g_b.balance, g_a.balance + g_b.balance);
    return 0;
}
