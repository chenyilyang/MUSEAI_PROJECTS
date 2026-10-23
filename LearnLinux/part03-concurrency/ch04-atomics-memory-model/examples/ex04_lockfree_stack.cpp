// ex04_lockfree_stack.cpp —— 无锁栈（Treiber stack）：CAS 循环的典型形态
// 全部同步只靠一个原子头指针：push/pop 都是"读旧值 → 算新值 → CAS 抢占，
// 失败说明有人抢先，带着被更新的旧值重试"。没有锁，没人会睡在临界区里。
// 本例的诚实边界（正文展开）：
//   1) pop 出的节点故意不 delete——别的线程可能还握着这个指针在重试，
//      提前释放就是 use-after-free；工业级回收要 hazard pointer / epoch 等机制。
//   2) ABA 问题在本结构+不复用内存的前提下暂不发作，复用内存即可能中招。
// CAS 先用默认（seq_cst）序求稳，内存序精细化不是本章的战场。
// 编译基线：C++11
#include <atomic>
#include <cstdio>
#include <thread>
#include <vector>

struct Node {
    int value;
    Node* next;
};

static std::atomic<Node*> g_head;

static void push(int v) {
    Node* n = new Node();
    n->value = v;
    n->next = g_head.load();
    while (!g_head.compare_exchange_weak(n->next, n)) {
        // CAS 失败：n->next 已被自动更新为最新头指针，循环重试
    }
}

static bool pop(int& out) {
    Node* old = g_head.load();
    while (old != NULL && !g_head.compare_exchange_weak(old, old->next)) {
        // CAS 失败：old 已被自动更新，继续试
    }
    if (old == NULL) return false;   // 栈已空
    out = old->value;
    return true;                     // old 故意不 delete（见文件头注释）
}

int main() {
    g_head.store(NULL);              // C++11 的 atomic 默认构造不保证清零，显式置空

    const int kThreads = 4;
    const int kPerThread = 250;
    // 入栈：线程 t 压入 t*250+1 .. t*250+250，合计 1..1000
    {
        std::vector<std::thread> pool;
        for (int t = 0; t < kThreads; ++t)
            pool.push_back(std::thread([t] {
                for (int v = t * kPerThread + 1; v <= (t + 1) * kPerThread; ++v) push(v);
            }));
        for (size_t i = 0; i < pool.size(); ++i) pool[i].join();
    }
    // 出栈：4 个线程抢到空为止，各自记件数与总和
    long sums[kThreads] = {0, 0, 0, 0};
    int counts[kThreads] = {0, 0, 0, 0};
    {
        std::vector<std::thread> pool;
        for (int t = 0; t < kThreads; ++t)
            pool.push_back(std::thread([t, &sums, &counts] {
                int v;
                while (pop(v)) {
                    sums[t] += v;
                    ++counts[t];
                }
            }));
        for (size_t i = 0; i < pool.size(); ++i) pool[i].join();
    }
    int totalCount = 0;
    long totalSum = 0;
    for (int t = 0; t < kThreads; ++t) {
        totalCount += counts[t];
        totalSum += sums[t];
    }
    std::printf("popped = %d (expected 1000)\n", totalCount);
    std::printf("popped sum = %ld (expected 500500)\n", totalSum);
    return 0;
}
