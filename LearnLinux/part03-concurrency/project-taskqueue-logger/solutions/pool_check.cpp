// pool_check —— ThreadPool 验收程序（starter 与 solutions 共用同一份）
// 一查结果：20 个平方任务经 future 取回，总和精确。
// 二查关闭语义：投递 10 个慢任务后立刻 shutdown()（排空退出）——
//               shutdown 返回时 10 个任务必须全部执行完毕，一个不能少。
#include <atomic>
#include <chrono>
#include <cstdio>
#include <future>
#include <thread>
#include <vector>

#include "thread_pool.h"

int main() {
    ThreadPool pool(4);

    long sum = 0;
    try {
        std::vector<std::future<int> > futs;
        for (int i = 0; i < 20; ++i)
            futs.push_back(pool.submit([i] { return i * i; }));
        for (int i = 0; i < 20; ++i) sum += futs[i].get();
    } catch (const std::future_error& e) {
        std::printf("pool: broken future (%s)\n", e.what());
        return 1;
    }
    std::printf("pool: sum = %ld (expected 2470)\n", sum);

    std::atomic<int> done(0);
    for (int i = 0; i < 10; ++i) {
        pool.submit([&] {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            done.fetch_add(1);
        });
    }
    pool.shutdown();                          // 排空退出：返回即意味着队列做空
    std::printf("pool: drained %d/10 before shutdown returned\n", done.load());

    return (sum == 2470 && done.load() == 10) ? 0 : 1;
}
