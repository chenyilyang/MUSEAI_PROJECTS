// 练习 3 参考答案：用 atomic 状态机实现 run-once（一次性初始化）
// 状态机：0 未初始化 → 1 初始化中 → 2 完成。
// CAS 抢到 0→1 的人负责初始化；其余人自旋等到 2 才返回——
// 这样 ensureInit 返回时，每个调用者都能确信初始化已完成且只发生过一次。
#include <atomic>
#include <cstdio>
#include <thread>
#include <vector>

static std::atomic<int> g_state(0);
static std::atomic<int> g_initRuns(0);
static int g_config = 0;                    // 被初始化的普通数据

static void doInit() {
    g_config = 42;
    ++g_initRuns;
}

static void ensureInit() {
    int expected = 0;
    if (g_state.compare_exchange_strong(expected, 1, std::memory_order_acq_rel)) {
        doInit();                                     // 抢到资格：干活
        g_state.store(2, std::memory_order_release);  // 发布"完成"（连带 g_config 可见）
        return;
    }
    // 没抢到：等到完成（含 expected 被 CAS 更新为 1 的情况）
    while (g_state.load(std::memory_order_acquire) != 2) {
        std::this_thread::yield();
    }
}

int main() {
    std::vector<std::thread> pool;
    std::vector<int> seen(8, 0);
    for (int i = 0; i < 8; ++i)
        pool.push_back(std::thread([&, i] {
            ensureInit();
            seen[(size_t)i] = g_config;               // 每个人读到的都必须是 42
        }));
    for (size_t i = 0; i < pool.size(); ++i) pool[i].join();

    std::printf("init runs = %d\n", g_initRuns.load());   // 必须恰为 1
    bool all42 = true;
    for (size_t i = 0; i < seen.size(); ++i) if (seen[i] != 42) all42 = false;
    std::printf("all threads saw config = %s\n", all42 ? "42" : "WRONG");
    return (g_initRuns.load() == 1 && all42) ? 0 : 1;
}
