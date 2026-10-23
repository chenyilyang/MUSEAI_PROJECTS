// 练习 4 参考答案：原子票号发生器——fetch_add 的教科书用途
// 8 个线程各领 1000 张票，合并后必须是 0..7999 各一张、不重不漏。
// fetch_add 返回旧值这一个语义，就同时完成了"占号"和"推进"。
#include <algorithm>
#include <atomic>
#include <cstdio>
#include <thread>
#include <vector>

static std::atomic<long> g_nextTicket(0);

int main() {
    const int kThreads = 8;
    const int kPerThread = 1000;
    std::vector<std::vector<long> > buckets((size_t)kThreads);
    std::vector<std::thread> pool;
    for (int t = 0; t < kThreads; ++t)
        pool.push_back(std::thread([&, t] {
            for (int i = 0; i < kPerThread; ++i)
                buckets[(size_t)t].push_back(g_nextTicket.fetch_add(1, std::memory_order_relaxed));
        }));
    for (size_t i = 0; i < pool.size(); ++i) pool[i].join();

    std::vector<long> all;
    for (int t = 0; t < kThreads; ++t)
        all.insert(all.end(), buckets[(size_t)t].begin(), buckets[(size_t)t].end());
    std::sort(all.begin(), all.end());
    bool unique = (long)all.size() == (long)kThreads * kPerThread;
    for (size_t i = 0; unique && i < all.size(); ++i)
        if (all[i] != (long)i) unique = false;      // 排序后第 i 个必须恰为 i
    std::printf("tickets = %zu, unique = %s\n", all.size(), unique ? "yes" : "no");
    return unique ? 0 : 1;
}
