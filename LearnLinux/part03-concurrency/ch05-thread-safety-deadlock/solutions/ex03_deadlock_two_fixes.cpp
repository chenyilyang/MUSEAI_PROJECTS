// 练习 3 参考答案：同一个双向转账场景的两种修法
// 修法一（全局顺序）：不管转账方向，一律先锁地址更小的账户——
//           所有线程遵守同一全局顺序，循环等待无从形成。
// 修法二（std::lock)：一次原子地锁住两把，要么全拿到、要么一把不拿地重试——
//           从不"持有并等待"，四条件之一被釜底抽薪。
// 两组账户各做双向各 1000 次、每次 1 元：终局必须各回 1000、总额 2000。
#include <cstdio>
#include <mutex>
#include <thread>

struct Account {
    Account(long b) : balance(b) {}
    std::mutex m;
    long balance;
};

// —— 修法一 ——
static void transferOrdered(Account& from, Account& to, long amount) {
    Account* first = &from < &to ? &from : &to;    // 按地址定序，与方向无关
    Account* second = &from < &to ? &to : &from;
    std::lock_guard<std::mutex> lk1(first->m);
    std::lock_guard<std::mutex> lk2(second->m);
    from.balance -= amount;
    to.balance += amount;
}

// —— 修法二 ——
static void transferStdLock(Account& from, Account& to, long amount) {
    std::lock(from.m, to.m);
    std::lock_guard<std::mutex> lk1(from.m, std::adopt_lock);
    std::lock_guard<std::mutex> lk2(to.m, std::adopt_lock);
    from.balance -= amount;
    to.balance += amount;
}

template <typename F>
static bool runPair(F transferFn) {
    Account a(1000), b(1000);
    std::thread t1([&] { for (int i = 0; i < 1000; ++i) transferFn(a, b, 1); });
    std::thread t2([&] { for (int i = 0; i < 1000; ++i) transferFn(b, a, 1); });
    t1.join();
    t2.join();
    std::printf("A = %ld, B = %ld, total = %ld\n", a.balance, b.balance, a.balance + b.balance);
    return a.balance == 1000 && b.balance == 1000;
}

int main() {
    bool ok1 = runPair(transferOrdered);
    bool ok2 = runPair(transferStdLock);
    std::printf("ordered: %s, std::lock: %s\n", ok1 ? "OK" : "FAIL", ok2 ? "OK" : "FAIL");
    return (ok1 && ok2) ? 0 : 1;
}
