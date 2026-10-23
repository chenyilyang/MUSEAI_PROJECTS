// 练习 3 参考答案：多线程分段求和，与单线程结果对照
#include <cstdio>
#include <thread>
#include <vector>

static void partialSum(long lo, long hi, long* slot) {   // [lo, hi)
    long s = 0;
    for (long i = lo; i < hi; ++i) s += i;
    *slot = s;
}

int main() {
    const long kTotal = 1000000;          // 求 1..1000000
    const int kThreads = 4;
    std::vector<long> slots((size_t)kThreads, 0);
    std::vector<std::thread> pool;
    long chunk = (kTotal + 1) / kThreads; // 1..1000000 共 1000000 个数
    for (int i = 0; i < kThreads; ++i) {
        long lo = 1 + i * chunk;
        long hi = lo + chunk;
        if (hi > kTotal + 1) hi = kTotal + 1;
        pool.push_back(std::thread(partialSum, lo, hi, &slots[(size_t)i]));
    }
    for (size_t i = 0; i < pool.size(); ++i) pool[i].join();
    long total = 0;
    for (int i = 0; i < kThreads; ++i) total += slots[(size_t)i];
    std::printf("parallel total = %ld\n", total);          // 500000500000

    long single = 0;
    for (long i = 1; i <= kTotal; ++i) single += i;
    std::printf("single total   = %ld\n", single);         // 应当一致
    return total == single ? 0 : 1;
}
